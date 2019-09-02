
#include "config_manager.h"

int send_value_to_client(void *value, size_t size, ClientData *data);

// Creates and returns a config structure
Config *init_config() {
	Config *config = (Config*) malloc(sizeof(Config));
	config->key_subscribers = create_hashmap();
	config->key_values = create_hashmap();
	pthread_mutex_init(&config->config_lock, NULL);
	return config;
}

// Adds a client to the list of subscribers for a given key and sends the current value to that client
void add_subscriber(Config *config, char *key, ClientData *data) {
	size_t value_size = 0;
	pthread_mutex_lock(&config->config_lock);
	void *value = get_value(config->key_values, key, &value_size);
	if (value == NULL && value_size == 0) {
		value = "<null>";
		value_size = 7;
		// Adding new key to the list with <null> as the value so we never have keys in subscriber list but not actual config.
		set_value(config->key_values, key, value, value_size);
	}
	if (send_value_to_client(value, value_size, data) > 0) {
		// If the send is successful, we add the client that requested a subscribe to the list of subscribed clients.
		size_t size = 0;
		ClientSlot *prev_client_list = (ClientSlot*) get_value(config->key_subscribers, key, &size); 	// NP
		ClientSlot *client_list = add_client_to_list(data, prev_client_list);							// NP
		set_value(config->key_subscribers, key, client_list, sizeof(ClientSlot));						// NP
		prev_client_list = (ClientSlot*) get_value(config->key_subscribers, key, &size);				//
		print_client_list(prev_client_list); 															// SEGFAV: PROBLEM CALL when add is first
	}
	pthread_mutex_unlock(&config->config_lock);
}

// Sets the value for a given key and broadcasts that value to all the subscribed clients
void set_config_value(Config *config, char *key, void* value, size_t size) {
	printf("Setting key %s to value new value", key);
	pthread_mutex_lock(&config->config_lock);
	set_value(config->key_values, key, value, size);
	size_t _size;
	send_to_all_clients_in_list(get_value(config->key_subscribers, key, &_size), value, size);
	pthread_mutex_unlock(&config->config_lock);
}

// Saves the config to a persistent file
void save_persistent_config(Config *config) {
	fprintf(stderr, "[WARNING]: config_manager:save_persistent_config/1 not implemented");
	pthread_mutex_lock(&config->config_lock);
	pthread_mutex_unlock(&config->config_lock);
}

// Searches through all keys in subscribed clients map to eliminate clients matching *data
void remove_subscriber(Config *config, ClientData *data) {
	size_t num_keys;
	char **all_keys = get_keys(config->key_subscribers, &num_keys);

	printf("Keys[%d]: ", num_keys);
	print_string_list(all_keys, num_keys);

	pthread_mutex_lock(&config->config_lock);
	printf("Removing subscriber %d\n", data->socket_id);
	for (int i = 0; i < num_keys; i++) {
		size_t slot_pointer_size;
		ClientSlot *client_list = get_value(config->key_subscribers, all_keys[i], &slot_pointer_size);
		printf("Subscribers for \"%s\":\n ", all_keys[i]);
		print_client_list(client_list);
		ClientSlot *new_client_list = remove_client_data_from_list(data, client_list);
		printf("Subscribers for \"%s\" after removal:\n", all_keys[i]);
		print_client_list(new_client_list);
		set_value(config->key_subscribers, all_keys[i], new_client_list, sizeof(ClientSlot)); // PROBLEM CALL when remove is first
	}
	pthread_mutex_unlock(&config->config_lock);

	free_string_list(all_keys, num_keys);
}

// Checks for dead clients and removes them from the subscriber list if they exist
void reap_dead_subscribers() {

}

// Saves config to persistent storage and deletes in memory copy
void clean_config(Config *config) {
	fprintf(stderr, "[WARNING]: config_manager:clean_config/1 not implemented");
	save_persistent_config(config);
	pthread_mutex_lock(&config->config_lock);
	free_hashmap(config->key_subscribers);
	free_hashmap(config->key_values);
	pthread_mutex_unlock(&config->config_lock);
	pthread_mutex_destroy(&config->config_lock);
}

// Private

int send_value_to_client(void *value, size_t size, ClientData *data) {
	return send_client_data(data, value, size);
}
