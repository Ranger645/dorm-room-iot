
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
