
#include "server_runtime.h"


// Helper functions
uint32_t get_ip_address(ClientData *client);
void print_clients();
uint32_t get_my_ip_address(int sockfd);
void print_ip_address(uint32_t ip);


// The main function:
int main(int argc, char *argv[]) {

	// Initializing configuration:
	configuration = init_config();

	head = NULL;
	fd_set fd_select;
	struct timeval timeout;
	int port = SERVER_PORT;

	if (argc > 1)
		port = atoi(argv[1]);
	
	// Setting up the server to allow port reuse, and for accepts to be non-blocking.	
	server_id = create_server(port);
	if (setsockopt(server_id, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
		printf("setsockopt(SO_REUSEADDR) failed\n");
	printf("Created server on port %d\n", port);

	pthread_mutex_init(&client_list_access_lock, NULL);

	// Starting udp thread:
	pthread_create(&udp_thread, NULL, udp_handler, (void*) &continue_server);

	signal(SIGINT, close_server);

	// While loop for accepting clients into the server:
	while (continue_server) {
	
		FD_ZERO(&fd_select); /* clear the set */
		FD_SET(server_id, &fd_select); /* add our file descriptor to the set */

		// Waiting for the server to have an incoming client:
		timeout.tv_sec = SERVER_TIMEOUT;
		timeout.tv_usec = 0;
		int sel_option = select(server_id + 1, &fd_select, NULL, NULL, &timeout);

		// Only accepting a client if the server is still supposed to be running:
		if (continue_server) {
			reap_clients();
			if (FD_ISSET(server_id, &fd_select)) {
				struct sockaddr_in *client = malloc(sizeof(struct sockaddr_in));
				socklen_t client_size = sizeof(*client);

				int client_id = accept(server_id, (struct sockaddr*) client, &client_size);
				check_error(client_id);

				ClientData *client_storage = create_client(client_id, client);
				start_client(client_storage);
			}
		}

		// If a select error occurs, then we close.
		if (sel_option < 0) {
			fprintf(stderr, "\nSelect error, closing...");
			break;
		}

	}

	close_server(&continue_server);
	return 0;
}

// Creates a server and returns its socket id.
int create_server(int port) {
    int sid = socket(PF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    int status = bind(sid, (struct sockaddr*)&addr, sizeof(addr));
    check_error(status);

    status = listen(sid, CLIENT_QUEUE_LEN);
    check_error(status);

    return sid;
}

// Handling exits...
void close_server() {
	printf("\n");
	continue_server = 0;
	
	// Closing all the client connections:
	ClientSlot *cur = head;
	while (cur) {
		ClientSlot *prev = cur;
		cur = cur->next;
		kill_client(prev);
	}
	
	// Closing the server:
	close(server_id);

	// Waiting for the udp thread to exit
	pthread_join(udp_thread, NULL);
	pthread_mutex_destroy(&client_list_access_lock);

	fprintf(stderr, "\nServer exited normally.\n");
	exit(0);
}

void *udp_handler(void *args) {

	// Establishing udp listener socket
	int sockfd;
	char buffer[COMMAND_MAX_SIZE];
	struct sockaddr_in servaddr, cliaddr; 
	
	// Creating socket file descriptor 
	check_error(sockfd = socket(AF_INET, SOCK_DGRAM, 0));
	printf("Created UDP handler thread on FD %d\n", sockfd);

	uint32_t ip = get_my_ip_address(sockfd);
	print_ip_address(ip);
	
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	
	// Filling server information 
	servaddr.sin_family    = AF_INET; // IPv4 
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	servaddr.sin_port = htons(SERVER_PORT); 
	
	// Bind the socket with the server address 
	check_error(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)));

	// Setting up polling the file descriptor
	struct pollfd fd_struct;
	fd_struct.fd = sockfd;
	fd_struct.events = POLLIN;

	while (continue_server) {

		if (poll(&fd_struct, 1, 500) > 0) {
			unsigned int len;
			int n;
			n = recvfrom(sockfd, (char*) buffer, COMMAND_MAX_SIZE, 0, (struct sockaddr*) &cliaddr, &len); 
			buffer[n] = '\0';

			if (n >= 10 && strncmp(buffer, "ip_address", 10) == 0) {
				sendto(sockfd, (const char*) &ip, 4, 0, (const struct sockaddr*) &cliaddr, len);
				printf("Server IP address request received, responded with IP address bytes: %04u.\n", ip);
			}
		}
	}

	// Cleaning up the udp listener socket fd
	close(sockfd);
	return NULL;
}

void start_client(ClientData *config) {
	pthread_create(&(config->thread_id), NULL, handle_client, (void*) config);
}

// Allocates the memory for a client, adds the client to the list, and then starts the thread with this client.
ClientData *create_client(int client_id, struct sockaddr_in *client) {
	// Creating the actual client data structure itself
	ClientData *data = malloc(sizeof(ClientData));
	pthread_mutex_t *mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
	data->socket_id = client_id;
	data->client = client;
	data->continue_client = 1;
	data->client_dead = 0;
	data->lock_id = mutex;
	
	// Creating the spot in the list for the client to reside in
	ClientSlot *slot = malloc(sizeof(ClientSlot));
	slot->data = data;
	slot->prev = NULL;
	slot->next = NULL;

	// Adding the new client to the list of clients
	if (! head) {
		head = slot;
	} else {
		ClientSlot *cur = head;
		while (cur->next)
			cur = cur->next;
		slot->prev = cur;
		cur->next = slot;
	}
	
	printf("Created Client with ip %" PRIu32 " FD: %u\n", client->sin_addr.s_addr, client_id);
	return data;
}

// Looks for terminated clients and deallocates them. 
// Returns the number of clients reaped.
int reap_clients() {
	print_clients();
	
	ClientSlot *slot = head;
	int reaped = 0;
	while (slot) {
		ClientSlot *to_destroy = slot;
		slot = slot->next;
		if (to_destroy->data->client_dead) {
			fprintf(stderr, "Reaping client on FD: %u\n", to_destroy->data->socket_id);
			reaped += kill_client(to_destroy);
		}
	}
	return reaped;
}

// Kills the client from an exterior source and allows it to exit normally.
int kill_client(ClientSlot *slot) {
	
	// Telling the client thread to exit.
	end_client(slot->data);

	// Waiting for the client to exit.
	// This will clean up the thread args data and close the socket
	pthread_join(slot->data->thread_id, NULL);

	// Freeing the client data:
	free(slot->data);

	// Removing the client connection from the list of clients
	if (slot->prev)
		slot->prev->next = slot->next;
	if (slot->next)
		slot->next->prev = slot->prev;
	if (slot == head)
		head = slot->next;
	free(slot);
	slot = NULL;

	return 1;
}

// Handles an incoming client connection.
void *handle_client(void *targs) {
	pthread_cleanup_push(handle_client_cleanup, targs);
	ClientData *args = (ClientData*) targs;

	while(args->continue_client) {
		struct pollfd fd_struct;
		fd_struct.fd = args->socket_id;
		fd_struct.events = POLLIN | POLLRDHUP;
		int poll_result = poll(&fd_struct, 1, 500);

		if (poll_result < 0) {
			printf("Poll errorred with %d, disconnecting FD %d\n", poll_result, args->socket_id);
			break;
		}

		// Checking for closed FD or error:
		if ((POLLERR | POLLHUP | POLLRDHUP) & fd_struct.revents) {
			printf("Client error or socket close 0x%02x, disconnecting FD %d\n", fd_struct.revents, args->socket_id);
			break;
		}

		if (args->continue_client && poll_result > 0) {
			char buf[COMMAND_MAX_SIZE + 1];
			for (int i = 0; i < COMMAND_MAX_SIZE + 1; i++)
				buf[i] = 0;
			int bytes = read(args->socket_id, buf, COMMAND_MAX_SIZE);
			
			if (bytes == 0) {
				printf("Read returned 0 bytes, disconnecting FD %d\n", args->socket_id);
				break;
			}
			buf[bytes] = 0;

			int arg_count = 0;
			char **command = space_parse(buf, &arg_count);

			printf("Client sent command: %s\n", buf);
			if (!strcmp(command[0], "execute")) {
				// Executes the given command as if it was a bash terminal and links the socket to the input and output
				// of the spawned bash processes
				cmd_execute(command, arg_count, args);
			} else  if (!strcmp(command[0], "client_restart")) {
				// Clients should send this command when they restart from unexpected shutdown to clean up any server
				// memory that is left around from the previous connection.
				// [WARNING]: This should only be used if the sending client is assigned a static IP address in the DHCP server.
				cmd_client_restart(command, arg_count, args);
			} else  if (!strcmp(command[0], "config_subscribe")) {
				// Adds this client as a subscriber for a given key
				cmd_config_subscribe(command, arg_count, args);
			} else  if (!strcmp(command[0], "config_set")) {
				// Sets a new value in the configuration
				cmd_set_config_value(command, arg_count, args);
			} else {
				printf("Invalid command: [%s]\n", buf);
			}

			for (int i = 0; i < arg_count; i++)
				free(*(command + i));
			free(command);
		}
	}
	pthread_cleanup_pop(1);
	return NULL;
}

void handle_client_cleanup(void *targs) {
	ClientData *client = (ClientData*) targs;
	close(client->socket_id);
	free(client->client);
	pthread_mutex_destroy(client->lock_id);
	free(client->lock_id);
	client->client_dead = 1;
	printf("Thread ended for FD %d\n", client->socket_id);
}

// Command handlers

void cmd_execute(char **command, int arg_count, ClientData *args) {
	int pid = fork();
	
	if (pid == 0) {
		// Child
		
		// Assigning the socket to stdin and stdout
		dup2(args->socket_id, 0);
		dup2(args->socket_id, 1);
		close(args->socket_id);

		// Constructing the exec call:
		char **exec_args = malloc((arg_count) * sizeof(char*));
		for (int i = 1; i < arg_count; i++) {
			exec_args[i - 1] = malloc((strlen(command[i]) + 1));
			strcpy(exec_args[i - 1], command[i]);
		}
		exec_args[arg_count - 1] = NULL;

		execvp(exec_args[0], exec_args);
	} else {
		// Parent
		int return_status;
		waitpid(pid, &return_status, 0);
		printf("Done executing bash command %s on client %d\n", command[0], args->socket_id);
	}
}

void cmd_client_restart(char **command, int arg_count, ClientData *args) {
	uint32_t ip = get_ip_address(args);
	ClientSlot *cur = head;
	while (cur) {
		ClientSlot *to_destroy = cur;
		cur = cur->next;
		if (to_destroy->data->socket_id != args->socket_id && get_ip_address(to_destroy->data) == ip) {
			printf("Client restart causing FD %d reaping\n", to_destroy->data->socket_id);
			end_client(to_destroy->data);
		}
	}
}

void cmd_config_subscribe(char **command, int arg_count, ClientData *args) {
	add_subscriber(configuration, command[1], args);
}

void cmd_set_config_value(char **command, int arg_count, ClientData *args) {
	set_config_value(configuration, command[1], (void*) command[2], strlen(command[2]) + 1);
}

// Helper Functions

uint32_t get_ip_address(ClientData *data) {
	return (uint32_t) (&data->client->sin_addr)->s_addr;
}

void print_clients() {
	ClientSlot *cur = head;
	fprintf(stderr, "Clients List: ");
	while (cur) {
		fprintf(stderr, "%u: %d; ", cur->data->socket_id, cur->data->continue_client);
		cur = cur->next;
	}
	fprintf(stderr, "\n");
}

uint32_t get_my_ip_address(int sockfd) {
	struct ifreq req;
	req.ifr_addr.sa_family = AF_INET;
    strncpy(req.ifr_name, "eth0", IFNAMSIZ - 1);
    ioctl(sockfd, SIOCGIFADDR, &req);
	uint32_t ip = (uint32_t) (&((struct sockaddr_in*) &req.ifr_addr)->sin_addr)->s_addr;
	return ip;
}

void print_ip_address(uint32_t ip) {
	char char_ip[4];
	char_ip[0] = (0x000000FF & ip);
	char_ip[1] = (0x0000FF00 & ip) >> 8;
	char_ip[2] = (0x00FF0000 & ip) >> 16;
	char_ip[3] = (0xFF000000 & ip) >> 24;
	printf("IP Address: %d.%d.%d.%d\n", char_ip[0], char_ip[1], char_ip[2], char_ip[3]);
}
