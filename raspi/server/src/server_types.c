
#include "server_types.h"

/*
 * Return values:
 * 1 = success
 * 0 = client is dead
 * -1 = error
 */
int send_client_data(ClientData *data, void *to_send, size_t size) {
    if (data->client_dead)
        return 0;

    int bytes_written = 0;
    while (bytes_written < size) {
        int result = (int) send(data->socket_id, to_send + bytes_written, size - bytes_written, 0);
        if (result < 0) {
            fprintf(stderr, "Socket send error on FD %d, closing connection. ", data->socket_id);
            check_error(result);
            end_client(data);
            return -1;
        }
        bytes_written += result;
    }
    return 1;
}

void end_client(ClientData *data) {
    pthread_mutex_lock(data->lock_id);
	data->continue_client = 0;
	pthread_mutex_unlock(data->lock_id);
}

// Handles any error in the sockets:
void check_error(int status) {
    if (status < 0) {
        printf("Socket error: [%s]\n", strerror(errno));
    }
}

// Client list api

void print_client_list(ClientSlot *list) {
	ClientSlot *cur = list;
	fprintf(stderr, "Clients List: ");
	while (cur) {
		fprintf(stderr, "[%u: %d, %d]; ", cur->data->socket_id, cur->data->continue_client, cur->data->client_dead);
		cur = cur->next;
	}
	fprintf(stderr, "\n");
}

ClientSlot *add_client_to_list(ClientData *data, ClientSlot *list) {
    // Creating the spot in the list for the client to reside in
	ClientSlot *slot = malloc(sizeof(ClientSlot));
	slot->data = data;
	slot->prev = NULL;
	slot->next = list;

    if (list != NULL) {
        list->prev = slot;
    }

    return slot;
}

// Maybe removes a client from the list if the client has its dead flag tripped
// returns 1 if the client was removed and 0 if nothing happened
ClientSlot *remove_client_from_list(ClientSlot *slot, ClientSlot *list) {
    if (slot->data->client_dead) {
        // Freeing the client data:
        free(slot->data);

        // Removing the client connection from the list of clients
        if (slot->prev)
            slot->prev->next = slot->next;
        if (slot->next)
            slot->next->prev = slot->prev;
        if (slot == list)
            list = slot->next;
        free(slot);
        slot = NULL;
        return list;
    }
    return list;
}

void send_to_all_clients_in_list(ClientSlot *list, void *to_send, size_t size) {
    while (list)
        if (! list->data->client_dead)
            send_client_data(list->data, to_send, size);
}
