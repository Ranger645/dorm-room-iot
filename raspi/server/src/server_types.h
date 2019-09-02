
#ifndef _SERVER_TYPES_H
#define _SERVER_TYPES_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>

typedef struct ClientData {
	struct sockaddr_in *client;
	int socket_id;
	char continue_client;
	char client_dead;
	pthread_t thread_id;
	pthread_mutex_t *lock_id; // for preventing dual access of continue_client
} ClientData;

typedef struct ClientSlot {
	ClientData *data;
	struct ClientSlot *prev;
	struct ClientSlot *next;
} ClientSlot;

int send_client_data(ClientData *data, void *to_send, size_t size); // makes sure all data is sent, if failure, closes client.
void end_client(ClientData *data); // sets continue_client to 0 with lock
void check_error(int status);

// Client list functions:
void print_client_list(ClientSlot *list);
ClientSlot *add_client_to_list(ClientData *data, ClientSlot *list);
ClientSlot *remove_client_data_from_list(ClientData *data, ClientSlot *list);
ClientSlot *remove_client_from_list(ClientSlot *slot, ClientSlot *list);
int compare_client_data(ClientData *a, ClientData *b);
void send_to_all_clients_in_list(ClientSlot *list, void *to_send, size_t size);

#endif
