
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
	printf("Adding FD %d as a subscriber to key %s", data->socket_id, key);
	set_value(config->key_subscribers, key, data, sizeof(ClientData));
	size_t value_size;
	void *value = get_value(config->key_values, key, &value_size);
	if (send_value_to_client(value, value_size, data) > 0)
		set_value(config->key_subscribers, key, add_client_to_list(data, get_value(config->key_subscribers, key, sizeof(ClientSlot))), sizeof(ClientSlot));
}

// Sets the value for a given key and broadcasts that value to all the subscribed clients
void set_config_value(Config *config, char *key, void* value, size_t size) {
	printf("Setting key %s to value new value", key);
	set_config_value(config->key_values, key, value, size);
	send_to_all_clients_in_list(get_value(config->key_subscribers, key, sizeof(ClientSlot)), value, size);
}

// Saves the config to a persistent file
void save_persistent_config(Config *config) {
	fprintf(stderr, "[WARNING]: config_manager:save_persistent_config/1 not implemented");
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
