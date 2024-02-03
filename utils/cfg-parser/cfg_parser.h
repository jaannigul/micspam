#pragma once

#include <stdint.h>

#define OUT 

enum ConfigErrors {
	NO_ERROR = 0,
	PARSER_MEMORY_ERROR,
	PARSER_CONFIG_INIT_ERROR,
	PARSER_FAILED_TO_OPEN_FILE,
	PARSER_KEY_IS_EMPTY_CHAR,
	PARSER_EXPECTED_EQUALS_CHAR,
	CONFIG_KEY_NOT_FOUND,
	CONFIG_MEMORY_ERROR,
	CONFIG_INCORRECT_VALUE_TYPE
};

struct Config;

/*
* Parses a simple 'K = V' formatted configuration file into memory.
* @param filename Full/relative path to the file
* @param err Optional error parameter for more specific errors
* @return NULL if parsing was unsuccessful, otherwise a malloced Config*
*/
struct Config* cfg_parse(char* filename, OUT enum ConfigErrors* err);

/*
* Saves the config into a file with the simple 'K=V' format
* @param cfg Pointer to a loaded config
* @param filename Full/relative path to the file
* @return NO_ERROR if the save was successful, otherwise an error code
*/
enum ConfigError cfg_save(struct Config* cfg, char* filename);

/*
* Closes a config and frees any memory taken up by it.
* @param config Pointer to the configuration struct.
*/
void cfg_close(struct Config** config);

/*
* The following functions get a specific value from the config.
* If the key is not present in the config, err* is set.
* If the parsed value type and the value user is asking doesn't match, the err* is set.
*
* @param cfg Pointer to the parsed configuration
* @param key Determines which value we need to get from the config
* @param err Optional error parameter for the two mentioned errors.
*/
int cfg_get_int(struct Config* cfg, char* key, OUT enum ConfigErrors* err);
float cfg_get_float(struct Config* cfg, char* key, OUT enum ConfigErrors* err);
const char* cfg_get_str(struct Config* cfg, char* key, OUT enum ConfigErrors* err);

/*
* The following functions sets a specific value for a key in the config
* If the key is not present in the config, a brand new key to value mapping is created
* If the key exists but the type is incorrect, err* is set.
*
* @param cfg Pointer to the parsed configuration
* @param key Determines which value we need to get from the config
* @param err Optional error parameter for the two mentioned errors.
*/
enum ConfigErrors cfg_set_int(struct Config* cfg, char* key, int value);
enum ConfigErrors cfg_set_float(struct Config* cfg, char* key, float value);
enum ConfigErrors cfg_set_str(struct Config* cfg, char* key, char* value);