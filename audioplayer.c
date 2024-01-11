#include "audioplayer.h"
#include "consts.h"

#include <Windows.h>

pthread_t soundPlayer;
pthread_mutex_t threadCreationLock;

_Bool setupAudioPlayer() {
	if (pthread_mutex_init(&threadCreationLock, NULL) != 0) {
		return FALSE;
	}

	return TRUE;
}

_Bool checkForSuffix(const char* str, const char* suffix)
{
	if (!str || !suffix)
		return 0;
	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	if (lensuffix > lenstr)
		return 0;
	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

// count how many audio files user has in a specific folder
int countFilesInDirectory(const char* path) {
	WIN32_FIND_DATA fileData;
	int filesTotal = 0;

	HANDLE hFile = FindFirstFile(TEXT(USER_AUDIO_FILES_PATH), &fileData);
	if (hFile == INVALID_HANDLE_VALUE)
		return filesTotal;

	if (GetFileAttributes(fileData.cFileName) & FILE_ATTRIBUTE_DIRECTORY == 0)
		filesTotal++;

	while (FindNextFile(hFile, &fileData) != 0)
		if (GetFileAttributes(fileData.cFileName) & FILE_ATTRIBUTE_DIRECTORY == 0)
			filesTotal++;

	return filesTotal;
}

const char** getUserAudioFiles(const char* path) {

}