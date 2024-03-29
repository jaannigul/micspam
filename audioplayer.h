#pragma once

#include <pthread.h>
#include <Windows.h>

enum AudioPlayerResponses {
	PLAYER_NO_ERROR = 0,
	PLAYER_THREAD_KILLED,
	PLAYER_COULDNT_FIND_AUDIO,
	PLAYER_THREAD_FAILED_TO_KILL
};

_Bool setupAudioPlayer();
int countFilesInDirectory(const char* path);
int getUserAudioFiles(const char* path, OUT const char** fileList);
int togglePlayingAudio(char* audioPath);