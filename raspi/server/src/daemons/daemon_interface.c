
#include "daemon_interface.h"

// private
void daemon_send_to_client(Daemon *daemon, int client_id, char* buffer, size_t size);

/*
 * Forks the process, establishes the pipes for communicating back and forth,
 * creates the daemon structure, and starts the thread to handle daemon commands.
 * command: string bash command to be run to start this daemon
 * flags: flags to determine restart method
 */
Daemon *start_daemon(char *command, int flags, ClientSlot *clients_list) {

    Daemon *daemon = NULL;
    int s2d[2];
    int d2s[2];
    pipe(s2d);
    pipe(d2s);

    pid_t pid = fork();
    if (pid == 0) {
        // Child
        close(s2d[1]);
        close(d2s[0]);
        dup2(s2d[0], 0);
        dup2(d2s[1], 1);
        close(s2d[0]);
        close(d2s[1]);

        int length;
        char **parsed_command = space_parse(command, &length);

        int result = execvp(parsed_command[0], parsed_command);
        if (result < 0)
            fprintf(stderr, "Error starting daemon.");
        return NULL;
    } else {
        // Parent
        close(s2d[0]);
        close(d2s[1]);

        daemon = (Daemon*) malloc(sizeof(Daemon));
        daemon->daemon_process = pid;
        daemon->output = s2d[1];
        daemon->input = d2s[0];
        daemon->restart_flags = flags;
        daemon->clients_list = clients_list;
    }

    pthread_create(&daemon->daemon_handler, 0, handle_daemon, (void*) daemon);
    return daemon;
}

// Sends '<client-id> <message>' to the daemon. This message must be null terminated string.
// The deamon will interpret this as being sent from <client-id>
void send_to_daemon(int client_id, char *message, Daemon *daemon) {
    char string_id[10];
    sprintf(string_id, "%d ", client_id);
    int message_len = strlen(string_id) + strlen(message) + 1;
    char prepended_message[message_len];
    write(daemon->output, prepended_message, message_len);
}

/*
 * The daemon handler thread has to take care of messages commands from the 
 * daemon itself
 */
void *handle_daemon(void *targs) {
    Daemon *daemon = (Daemon*) targs;
    while (kill(daemon->daemon_process, 0) >= 0) {
        struct pollfd fd_struct;
		fd_struct.fd = daemon->input;
		fd_struct.events = POLLIN;
		int poll_result = poll(&fd_struct, 1, 5000);
		if (poll_result < 0) {
			fprintf(stderr, "Daemon poll error occured.\n");
			break;
		}
		if ((POLLERR | POLLHUP | POLLRDHUP) & fd_struct.revents) {
			fprintf(stderr, "Daemon stdout pipe closed.\n");
			break;
		}

        if (poll_result > 0) {
            char buf[DAEMON_COMMAND_MAX_SIZE + 1];
			for (int i = 0; i < DAEMON_COMMAND_MAX_SIZE + 1; i++)
				buf[i] = 0;
			int bytes = read(daemon->input, buf, DAEMON_COMMAND_MAX_SIZE);

            printf("Received command %s of length %d from daemon.\n", buf, bytes);
            int command_word_count;
            char **command = space_parse(buf, &command_word_count);

            // Command handling logic
            if (command != NULL && command_word_count > 0) {
                if (! strcmp(command[0], "send_to_client") && command_word_count > 2) {
                    // send_to_client <client-id> <message>
                    int client_id = atoi(command[1]);
                    int message_len = 0;
                    for (int i = 2; i < command_word_count; i++)
                        message_len += strlen(command[i]) + 1;
                    char client_command[message_len];
                    int command_iter = 0;
                    for (int i = 2; i < command_word_count; i++) {
                        int word_size = strlen(command[i]);
                        for (int n = 0; n < word_size; n++)
                            client_command[command_iter++] = command[i][n];
                        if (i == command_word_count - 1)
                            client_command[command_iter++] = '\n';
                        else
                            client_command[command_iter++] = ' ';
                    }
                    daemon_send_to_client(daemon, client_id, client_command, message_len);
                }
            }
            free_string_list(command, command_word_count);
        }
    }

    kill(daemon->daemon_process, SIGTERM);
    int status;
    waitpid(daemon->daemon_process, &status, 0);
    return NULL;
}

void free_daemon(Daemon *daemon) {
    kill(daemon->daemon_process, SIGTERM);
    int status;
    waitpid(daemon->daemon_process, &status, 0);
    close(daemon->input);
    close(daemon->output);
    free(daemon);
}

// private

void daemon_send_to_client(Daemon *daemon, int client_id, char* buffer, size_t size) {
    ClientSlot *slot = daemon->clients_list;
    while(slot != NULL) {
        if (slot->data->client_id == client_id) {
            send_client_data(slot->data, (void*) buffer, size);
            return;
        }
        slot = slot->next;
    }
    fprintf(stderr, "Not able to send to client with that id. Client doesn't exist.\n");
}
