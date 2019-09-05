
#ifndef _SERVER_RUNTIME_H
#define _SERVER_RUNTIME_H

#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <poll.h>

#include "server_types.h"
#include "config/config_manager.h"
#include "util/util_string.h"
#include "daemons/daemon_interface.h"

#define CLIENT_QUEUE_LEN 10
#define SERVER_PORT 10101
#define SERVER_TIMEOUT 5
#define CLIENT_TIMEOUT 5
#define COMMAND_MAX_SIZE 256

// For client close socket poll revent
#ifndef POLLRDHUP
#define POLLRDHUP 0x2000
#endif

pthread_mutex_t client_list_access_lock;

int server_id = -1;
char continue_server = 1;
ClientSlot *head;
int client_id_index = 0;

pthread_t udp_thread;

Config *configuration;

// Server functions
int create_server(int port);
void *udp_handler(void *args);
ClientData *create_client(int client_id, struct sockaddr_in *client);
void start_client(ClientData *config);
int kill_client(ClientSlot *slot);
int reap_clients();
void close_server();

// Client Functions
void *handle_client(void *targs);
void handle_client_cleanup(void *targs);
void cmd_execute(char **command, int arg_count, ClientData *args);
void cmd_client_restart(char **command, int arg_count, ClientData *args);
void cmd_config_subscribe(char **command, int arg_count, ClientData *args);
void cmd_set_config_value(char **command, int arg_count, ClientData *args);

#endif
