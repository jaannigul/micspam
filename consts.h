#pragma once

#define BUFFER_FRAMES 256

#define MAX_CHAR_BUF_LEN 2*MAX_PATH

#define REAL_MIC_DATA_PRIORITY 1
#define MICSPAM_DATA_PRIORITY 2

#define USER_AUDIO_FILES_PATH ".\\audiosamples"
#define USER_AUDIO_FILES_PATH_WILDCARD ".\\audiosamples\\*"
#define USER_LAST_DEVICE_IDS "lastdevices.txt" 
static const char* ALLOWED_AUDIO_TYPES[] = {
	"wav",
	"mp3"
};