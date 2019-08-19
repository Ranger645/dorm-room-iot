
#include "hashmap.h"

HashMap *create_hashmap() {
    HashMap *map = (HashMap*) malloc(sizeof(HashMap));
    for (int i = 0; i < NUM_BUCKETS; i++)
        map->buckets[i] = NULL;
    return map;
}

void set_value(HashMap *map, char *key, void *value, size_t n) {
    Slot *new_slot = (Slot*) malloc(sizeof(Slot));
    char *new_key = (char*) malloc(sizeof(char) * strlen(key) + 1);
    strcpy(new_key, key);
    void *new_value = malloc(n);
    memcpy(new_value, value, n);
    new_slot->key = new_key;
    new_slot->value = new_value;
    new_slot->size = n;
    new_slot->next = NULL;

    unsigned long bucket = hash(key) % NUM_BUCKETS;
    Slot *cur_slot = map->buckets[bucket];
    if (cur_slot == NULL) {
        map->buckets[bucket] = new_slot;
    } else {
        while (cur_slot->next)
            if (! strcmp(cur_slot->key, key)) {
                // Updating the current value
                free(new_slot);
                free(new_slot->key);
                free(cur_slot->value);
                cur_slot->value = new_value;
                cur_slot->size = n;
                return;
            } else
                cur_slot = cur_slot->next;
        cur_slot->next = new_slot;
    }
}

void *get_value(HashMap *map, char *key, size_t *size) {
    unsigned long bucket = hash(key) % NUM_BUCKETS;
    Slot *cur_slot = map->buckets[bucket];
    if (cur_slot != NULL) {
        while(cur_slot)
            if (! strcmp(cur_slot->key, key))
                return cur_slot->value;
            else
                cur_slot = cur_slot->next;
    }
    return NULL;
}

void *remove_value(HashMap *map, char *key, size_t *size) {
    unsigned long bucket = hash(key) % NUM_BUCKETS;
    Slot *cur_slot = map->buckets[bucket];
    Slot *previous_slot = NULL;
    if (cur_slot != NULL)
        while(cur_slot) {
            if (! strcmp(cur_slot->key, key)){
                void *value = cur_slot->value;
                if (previous_slot == NULL)
                    map->buckets[bucket] = cur_slot->next;
                else
                    previous_slot->next = cur_slot->next;
                free(cur_slot->key);
                free(cur_slot->value);
                free(cur_slot);
                return value;
            } else {
                previous_slot = cur_slot;
                cur_slot = cur_slot->next;
            }
        }
    return NULL;
}

int free_hashmap(HashMap *map) {
    int keys_freed = 0;
    Slot *cur = NULL;
    Slot *prev = NULL;
    for (int i = 0; i < NUM_BUCKETS; i++) {
        cur = map->buckets[i];
        while (cur) {
            prev = cur;
            cur = cur->next;
            free(prev->key);
            free(prev->value);
            free(prev);
            keys_freed++;
        }
    }
    free(map);
    return keys_freed;
}

unsigned long hash(char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
