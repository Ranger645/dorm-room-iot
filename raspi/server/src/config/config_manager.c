
#include "config_manager.h"

int send_value_to_client(void *value, size_t size, ClientData *data);

// Creates and returns a config structure
Config *init_config() {
	Config *config = (Config*) malloc(sizeof(Config));
	config->key_subscribers = create_hashmap();
	config->key_values = create_hashmap();
	return config;
}

// Adds a client to the list of subscribers for a given key and sends the current value to that client
void add_subscriber(Config *config, char *key, ClientData *data) {
	set_value(config->key_subscribers, key, data, sizeof(ClientData));
	size_t value_size = 0;
	void *value = get_value(config->key_values, key, &value_size);
	if (value == NULL && value_size == 0) {
		value = "<null>";
		value_size = 7;
	}
	if (send_value_to_client(value, value_size, data) > 0) {
		// If the send is successful, we add the client that requested a subscribe to the list of subscribed clients.
		size_t size = 0;
		ClientSlot *client_list = add_client_to_list(data, (ClientSlot*) get_value(config->key_subscribers, key, &size));
		set_value(config->key_subscribers, key, client_list, sizeof(ClientSlot));
	}
}

// Sets the value for a given key and broadcasts that value to all the subscribed clients
void set_config_value(Config *config, char *key, void* value, size_t size) {
	printf("Setting key %s to value new value", key);
	set_value(config->key_values, key, value, size);
	size_t _size;
	send_to_all_clients_in_list(get_value(config->key_subscribers, key, &_size), value, size);
}

// Saves the config to a persistent file
void save_persistent_config(Config *config) {
	fprintf(stderr, "[WARNING]: config_manager:save_persistent_config/1 not implemented");
}

// Checks for dead clients and removes them from the subscriber list if they exist
void reap_dead_subscribers() {
	
}

// Saves config to persistent storage and deletes in memory copy
void clean_config(Config *config) {
	fprintf(stderr, "[WARNING]: config_manager:clean_config/1 not implemented");
	save_persistent_config(config);
}

// Private

int send_value_to_client(void *value, size_t size, ClientData *data) {
	return send_client_data(data, value, size);
}
