#pragma once
#define BUFFER_FRAMES 256

#define REAL_MIC_DATA_PRIORITY 1
#define MICSPAM_DATA_PRIORITY 2

#define USER_AUDIO_FILES_PATH_WILDCARD ".\\audiosamples\\*"
#define MAX_PATH_LENGTH 256
static const char* ALLOWED_AUDIO_TYPES[] = {
	"wav",
	"mp3"
};