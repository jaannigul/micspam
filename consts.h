#pragma once
#define BUFFER_FRAMES 4096

#define REAL_MIC_DATA_PRIORITY 1
#define MICSPAM_DATA_PRIORITY 2

#define USER_AUDIO_FILES_PATH "./audiosamples"

static const char* ALLOWED_AUDIO_TYPES[] = {
	"wav",
	"mp3"
};