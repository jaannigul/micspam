#include "cfg_parser.h"
#include "hashmap.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#define SEED0 0x811C9DC5
#define SEED1 0x1000193

enum ValueType {
	INT,
	FLOAT,
	STR
};

struct Config {
	struct hashmap* key_to_value;
};

struct Value {
	enum ValueType type;
	void* value;
};

struct HashmapItem {
	char* key;
	struct Value* value;
};

int hashmap_compare_func(const void* a, const void* b, void* udata) {
	const struct HashmapItem* ua = a;
	const struct HashmapItem* ub = b;
	return strcmp(ua->key, ub->key);
}

uint64_t hashmap_user_hash(const void* item, uint64_t seed0, uint64_t seed1) {
	const struct HashmapItem* user = item;
	return hashmap_sip(user->key, strlen(user->key), seed0, seed1);
}

void free_multiple(void* arg1, ...)
{
	va_list args;
	void* vp;
	free(arg1);
	va_start(args, arg1);
	while ((vp = va_arg(args, void*)) != 0)
		free(vp);
	va_end(args);
}

/* 
 * Returns a malloced char* that contains the characters of one line in the file.
 * @param file Pointer to the opened file stream
 * @param err Optional pointer for storing error codes
 * @return Returns a malloced char* with all characters that were read on the single line, including the newline char
*/
char* read_line(FILE* file, enum ConfigErrors* err) {
	int curr_line_len = 16;
	int line_start_pos = ftell(file);
	char* line = malloc(curr_line_len);
	if (!line) {
		if(err) *err = PARSER_MEMORY_ERROR;
		return NULL;
	}

	while (fgets(line, curr_line_len, file) != NULL) {
		int num_chars_read = strlen(line);

		if (num_chars_read < curr_line_len-1)
			break;

		if (num_chars_read == curr_line_len - 1 && line[curr_line_len - 1] == '\n')
			break;

		// expand the str in case we didnt read the line completely
		curr_line_len *= 2;
		char* reallocatedLine = realloc(line, curr_line_len);
		if (!reallocatedLine) {
			free(line);
			if (err) *err = PARSER_MEMORY_ERROR;
			return NULL;
		}

		line = reallocatedLine;
		fseek(file, line_start_pos, SEEK_SET);
	}

	return line;
}

/*
* Checks whether there is only one occurrence of a comma/point in the string and whether it is between two numbers
* @param key String without any whitespaces to look for the valid comma
* @return Returns true if the statement is correct, false otherwise
*/
bool is_comma_valid(char* str) {
	int len = strlen(str);
	int occurrences = 0;
	for (int pos = 0; pos < len; pos++)
		if (str[pos] == ',' || str[pos] == '.')
			occurrences++;

	if (occurrences != 1)
		return false;

	char* comma_loc = (char*)((uintptr_t)strchr(str, (int)'.') | (uintptr_t)strchr(str, (int)','));
	return comma_loc-1 > str && comma_loc + 1 <= str+len && isdigit(*(comma_loc - 1)) && isdigit(*(comma_loc + 1));
}

/*
* Checks whether there is only one occurrence of a plus/minus in the string and checks if the number sign is written correctly before a number without anything preceeding it
* @param key String without any whitespaces, where we will search for the plus/minus
* @return Returns true if the statement is correct, false otherwise
*/
bool is_number_sign_valid(char* str) {
	int len = strlen(str);
	int occurrences = 0;
	for (int pos = 0; pos < len; pos++)
		if (str[pos] == '+' || str[pos] == '-')
			occurrences++;

	if (occurrences == 0)
		return true;
	if (occurrences > 2)
		return false;

	char* sign_loc = (uintptr_t)strchr(str, (int)'+') | (uintptr_t)strchr(str, (int)'-');
	return sign_loc <= str+len && isdigit(*(sign_loc + 1));
}

bool strcontains(char* str, char letter) {
	int len = strlen(str);
	for (int i = 0; i < len; i++)
		if (str[i] == letter)
			return true;

	return false;
}

/*
* Determines the key's value type so we can convert it to a proper value and return an error if the code tries to convert a value incorrectly.
* @param key An array of characters that were left after '='
* @return The type of the value which seemed to fit it the best
*/
enum ValueType determine_value_type(char* value) {
	int max_search_len = strlen(value)-1;
	// find first nonspace char from left
	while (max_search_len >= 0) {
		if (!isspace(value[max_search_len]))
			break;

		max_search_len--;
	}

	enum ValueType type = STR;
	int first_nonwhitespace_char_pos = -1;
	int alphabetic_chars = 0, numeric_chars = 0, minuses_or_pluses = 0, commas_or_points = 0, whitespaces_after_first_nonwsp = 0;

	// make char observations required later for making the type decision
	for (int pos = 0; pos <= max_search_len; pos++) {
		if (!isspace(value[pos]) && first_nonwhitespace_char_pos == -1)
			first_nonwhitespace_char_pos = pos;

		if (isspace(value[pos]) && first_nonwhitespace_char_pos != -1)
			whitespaces_after_first_nonwsp++;

		if (isalpha(value[pos]) || strcontains("!\"#$%&'()*/:;<=>?@[\\]^_`{|}~", value[pos]))
			alphabetic_chars++;

		if (isdigit(value[pos]))
			numeric_chars++;

		if (value[pos] == '.' || value[pos] == ',')
			commas_or_points++;
	}

	if (whitespaces_after_first_nonwsp == 0 && alphabetic_chars == 0) { // numbers shall not have any alphabetic characters in them or whitespaces
		char* first_nonwhitespace_char = value + first_nonwhitespace_char_pos;
		if (is_number_sign_valid(first_nonwhitespace_char)) {
			if (is_comma_valid(first_nonwhitespace_char))
				type = FLOAT;
			else if(commas_or_points == 0)
				type = INT;
		}
	}

	return type;
}

/*
* Removes all whitespace characters from the start and end of a string
* @param str String from which to remove the whitespaces from
* @return New malloced string with no leading or trailing whitespaces.
*/
char* lrtrim(char* str) {
	int l = 0, r = strlen(str)-1;
	while (isspace(str[l]) && l <= r)
		l++;

	while (isspace(str[r]) && r > l)
		r--;

	int dest_size = r - l + 1;
	char* new_str = malloc(dest_size + 1);
	if (!new_str)
		return NULL;

	strncpy_s(new_str, dest_size + 1, str, dest_size); 

	return new_str;
}

/*
* Parses a config file line 'K = V' into a hashmap value, which can later be retrieved from the config. All whitespaces before and after K will be trimmed.
* Everything before the first equals sign will be considered a key, and everything after that will be considered the value.
* @param line Line to parse
* @param err Optional pointer for storing error codes
*/
struct HashmapItem* parse_line(char* line, enum ConfigErrors* err) {
	char* separator_pos = strchr(line, (int)'=');
	if (separator_pos == NULL) {
		if(err) *err = PARSER_EXPECTED_EQUALS_CHAR;
		return NULL;
	}
	if (separator_pos == line) {
		if(err) *err = PARSER_KEY_IS_EMPTY_CHAR;
		return NULL;
	}

	// momentarily replace separator with null terminator so that the trimmed key wont consist of the value
	*separator_pos = '\0';
	char* key_trimmed = lrtrim(line);
	*separator_pos = '=';

	if (strlen(key_trimmed) == 0) {
		free(key_trimmed);
		if (err) *err = PARSER_KEY_IS_EMPTY_CHAR;
		return NULL;
	}

	char* str_value = separator_pos + 1;
	if (!key_trimmed) {
		if(err) *err = PARSER_MEMORY_ERROR;
		return NULL;
	}

	// malloc memory based on the type deferred from str
	enum ValueType value_type = determine_value_type(str_value);
	void* value = NULL;
	switch (value_type) {
	case INT:
		value = malloc(sizeof(int));
		if (value)
			*(int*)value = atoi(str_value);
		break;

	case FLOAT:
		value = malloc(sizeof(float));
		if (value)
			*(float*)value = atof(str_value);
		break;

	case STR: // copy our key over to a new string as the file line will get deallocated
		int len = strlen(str_value);
		if (str_value[len - 1] == '\n') len--;

		value = malloc(len * sizeof(char) + 1);
		if (value) {
			strncpy_s(value, len+1, str_value, len);
			((char*)value)[len] = '\0';
		}

		break;
	}

	if (!value) {
		free(key_trimmed);
		if(err) *err = PARSER_MEMORY_ERROR;
		return NULL;
	}

	struct HashmapItem* item = malloc(sizeof(struct HashmapItem));
	if (!item) {
		free_multiple(value, key_trimmed);
		if(err) *err = PARSER_MEMORY_ERROR;
		return NULL;
	}

	struct Value* hashmapItemValue = malloc(sizeof(struct Value));
	if (!hashmapItemValue) {
		free_multiple(value, key_trimmed, item); // i think i understand now why unique_ptr or shared_ptr was invented...
		if(err) *err = PARSER_MEMORY_ERROR;
		return NULL;
	}

	hashmapItemValue->type = value_type;
	hashmapItemValue->value = value;

	item->key = key_trimmed;
	item->value = hashmapItemValue;

	return item;
}

/*
* Internal function for freeing all hashmap elements. Called when hashmap_free is used.
*/
void cfg_internal_free(struct HashmapItem* item) {
	free_multiple(item->key, item->value->value, item->value, item);
}


struct Config* cfg_parse(char* filename, enum ConfigErrors* err) {
	struct hashmap* map = hashmap_new(sizeof(struct HashmapItem), 0, SEED0, SEED1, hashmap_user_hash, hashmap_compare_func, cfg_internal_free, NULL);
	if (!map) {
		if(err) *err = PARSER_CONFIG_INIT_ERROR;
		return NULL;
	}

	struct Config* config = malloc(sizeof(struct Config));
	if (!config) {
		if (err) *err = PARSER_CONFIG_INIT_ERROR;
		free(map);
		return NULL;
	}

	config->key_to_value = map;

	FILE* file;
	errno_t ferr = fopen_s(&file, filename, "r");
	if (!file) {
		if (err) *err = PARSER_FAILED_TO_OPEN_FILE;
		free_multiple(map, config);
		return NULL;
	}

	int linenr = 0;
	while (!feof(file)) {
		enum ConfigError local_err = NO_ERROR;
		char* line = read_line(file, &local_err);
		if (local_err != NO_ERROR) {
			if (err) *err = local_err;

			free_multiple(map, line, config);
			return NULL;
		}

		struct HashmapItem* item = parse_line(line, &local_err);
		if (!item) {
			if (err) *err = local_err;

			free_multiple(map, line, config);
			return NULL;
		}

		hashmap_set(map, item);

		free(line);
		linenr++;
	}

	return config;
}

void cfg_close(struct Config* config) {
	if (!config) return;

	hashmap_free(config->key_to_value);
	free(config);
}

int cfg_get_int(struct Config* cfg, char* key, enum ConfigErrors* err) {
	struct HashmapItem searchItem = { .key = key };
	const struct HashmapItem* found = hashmap_get(cfg->key_to_value, &searchItem);
	if (!found) {
		if (err) *err = CONFIG_KEY_NOT_FOUND;
		return 0;
	}

	if (found->value->type != INT) {
		if (err) *err = CONFIG_INCORRECT_VALUE_TYPE_FOR_GET;
		return 0;
	}

	return *(int*)found->value->value;
}

float cfg_get_float(struct Config* cfg, char* key, OUT enum ConfigErrors* err) {
	struct HashmapItem searchItem = { .key = key };
	const struct HashmapItem* found = hashmap_get(cfg->key_to_value, &searchItem);
	if (!found) {
		if (err) *err = CONFIG_KEY_NOT_FOUND;
		return 0;
	}

	if (found->value->type != INT) {
		if (err) *err = CONFIG_INCORRECT_VALUE_TYPE_FOR_GET;
		return 0;
	}

	return *(float*)found->value->value;
}
char* cfg_get_str(struct Config* cfg, char* key, OUT enum ConfigErrors* err) {
	struct HashmapItem searchItem = { .key = key };
	const struct HashmapItem* found = hashmap_get(cfg->key_to_value, &searchItem);
	if (!found) {
		if (err) *err = CONFIG_KEY_NOT_FOUND;
		return 0;
	}

	if (found->value->type != INT) {
		if (err) *err = CONFIG_INCORRECT_VALUE_TYPE_FOR_GET;
		return 0;
	}

	return (char*)found->value->value;
}