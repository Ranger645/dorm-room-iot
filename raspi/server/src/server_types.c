
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

    ssize_t bytes_written = 0;
    while (bytes_written < size) {
        ssize_t result = write(data->socket_id, to_send + bytes_written, size - bytes_written);
        if (result < 0) {
            fprintf("Socket send error on FD %d, closing connection.\n", data->socket_id);
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
    ClientSlot *head = list;

    // Creating the spot in the list for the client to reside in
	ClientSlot *slot = malloc(sizeof(ClientSlot));
	slot->data = data;
	slot->prev = NULL;
	slot->next = NULL;

	// Adding the new client to the list of clients
	if (! list) {
		head = slot;
	} else {
		ClientSlot *cur = list;
		while (cur->next)
			cur = cur->next;
		slot->prev = cur;
		cur->next = slot;
	}
    return head;
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
