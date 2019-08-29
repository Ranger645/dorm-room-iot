
/*
 * Defines functions and stuctures for creating hashmaps. Stores dynamically
 * allocated copies of the keys and values on the heap. Frees all memory when
 * values are removed or the map is freed.
 * 
 * Uses djb2 hashing explained here: http://www.cse.yorku.ca/~oz/hash.html
 */

#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_BUCKETS 128

typedef struct MapSlot {
    char *key;
    void *value;
    size_t size;
    struct MapSlot *next;
} Slot;

typedef struct HashMapStruct {
    Slot *buckets[NUM_BUCKETS];
} HashMap;

HashMap *create_hashmap();
void set_value(HashMap *map, char *key, void *value, size_t n);
void *get_value(HashMap *map, char *key, size_t *size);
void *remove_value(HashMap *map, char *key, size_t *size);
int free_hashmap(HashMap *map);
unsigned long hash(char *str);

#endif
