

#ifndef _CONFIG_MANAGER_H
#define _CONFIG_MANAGER_H

#include "../server_types.h"
#include "../util/hashmap.h"
#include "../util/util_string.h"

typedef struct ConfigStruct {
    HashMap *key_subscribers;
    HashMap *key_values;
    pthread_mutex_t config_lock;
} Config;

// Functions
Config *init_config();
void add_subscriber(Config *config, char *key, ClientData *data);
void set_config_value(Config *config, char *key, void* value, size_t size);
void save_persistent_config(Config *config);
void remove_subscriber(Config *config, ClientData *data);
void reap_dead_subscribers();
void clean_config(Config *config);

#endif
