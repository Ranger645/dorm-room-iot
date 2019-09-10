
#include "util_string.h"

char **space_parse(char *str, int *count) {

	// If the string is empty, return null:
	if (!strlen(str))
		return NULL;

	// Counting the number space deliminated sectors:
	char *iter = str;
	*count = 0;
	while (*iter) {
		if (isspace(*iter)) {
			while (*iter && isspace(*iter))
				iter++;
		} else {
			(*count)++;
			while (*iter && !isspace(*iter))
				iter++;
		}
	}
	
	// Initializing the list of strings:
	char **list = malloc((*count) * sizeof(char*));
	int current = 0;
	int current_size = 0;
	iter = str;
	char *name = malloc(1);
	*name = '\0';

	while (current < *count) {
		if (isspace(*iter)) {
			while (*iter && isspace(*iter))
				iter++;
		} else {
			while (*iter && !isspace(*iter)) {
				current_size++;
				char *concat = malloc(current_size + 1); // +1 for the \0
				char char_str[2];
				char_str[0] = *iter;
				char_str[1] = 0;
				strcpy(concat, name);
				strcat(concat, char_str);
				free(name);
				name = concat;
				iter++;
			}
			*(list + current) = name;
			name = malloc(1);
			*name = '\0';
			current++;
		}
	}
	free(name);
	return list;
}

char *string_join(char **string_list, size_t len, char *delim) {
	int size = (len - 1) * strlen(delim) + 1; // start at one for the null term
	for (int i = 0; i < len; i++) {
		size += strlen(string_list[i]);
	}
	char *str = (char*) malloc(sizeof(char) * size);
	str[0] = 0;
	for (int i = 0; i < len; i++) {
		strcat(str, string_list[i]);
		if (i != len - 1)
			strcat(str, delim);
	}
	return str;
}

void free_string_list(char **string_list, size_t len) {
	for (int i = 0; i < len; i++)
		free(string_list[i]);
	free(string_list);
}

void print_string_list(char **string_list, size_t len) {
	printf("[");
	for (int i = 0; i < len; i++)
		if (i == len - 1)
			printf("%s", string_list[i]);
		else
			printf("%s, ", string_list[i]);
	printf("]\n");
}
