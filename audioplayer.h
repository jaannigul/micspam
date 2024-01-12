#pragma once

#include <pthread.h>
#include <Windows.h>

extern pthread_t soundPlayer;
extern pthread_mutex_t threadCreationLock;

enum AudioPlayerErrors {
	PLAYER_NO_ERROR = 0,
	PLAYER_COULDNT_FIND_AUDIO
};

_Bool setupAudioPlayer();
int countFilesInDirectory(const char* path);
int getUserAudioFiles(const char* path, OUT const char** fileList);