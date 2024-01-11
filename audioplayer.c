#include "audioplayer.h"
#include <Windows.h>

pthread_t soundPlayer;
pthread_mutex_t threadCreationLock;

_Bool setupAudioPlayer() {
	if (pthread_mutex_init(&threadCreationLock, NULL) != 0) {
		return FALSE;
	}

	return TRUE;
}

// count how many audio files user has in a specific folder
int countUserAudioFiles(const char* path) {

}

const char** getUserAudioFiles(const char* path) {

}