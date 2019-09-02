

#ifndef _UTIL_STRING_H
#define _UTIL_STRING_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

char **space_parse(char *str, int *count);
void free_string_list(char **string_list, size_t len);
void print_string_list(char **string_list, size_t len);

#endif