#include "audioplayer.h"
#include "consts.h"

#include <signal.h>
#include <stdio.h>

pthread_t soundPlayer;
_Bool threadShouldBeRunning = FALSE; // shitty workaround for stdatomic.h to check if our audio player thread is running

BOOL directoryExists(const char* path)
{
	DWORD dwAttrib = GetFileAttributes(path);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
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

_Bool isAllowedAudioFile(const char* filename) {
	for (int i = 0; i < _countof(ALLOWED_AUDIO_TYPES); i++) {
		if (checkForSuffix(filename, ALLOWED_AUDIO_TYPES[i]))
			return TRUE;
	}

	return FALSE;
}

_Bool setupAudioPlayer() {
	if (!directoryExists(USER_AUDIO_FILES_PATH))
		if (CreateDirectory(USER_AUDIO_FILES_PATH, NULL) == 0)
			return FALSE;

	return TRUE;
}


// count how many files user has in a specific folder
int countFilesInDirectory(const char* path) {
	WIN32_FIND_DATA fileData;
	int filesTotal = 0;

	HANDLE hFile = FindFirstFile(TEXT(USER_AUDIO_FILES_PATH), &fileData);
	if (hFile == INVALID_HANDLE_VALUE)
		return filesTotal;

	if ((GetFileAttributes(fileData.cFileName) & FILE_ATTRIBUTE_DIRECTORY) == 0)
		filesTotal++;

	while (FindNextFile(hFile, &fileData) != 0)
		if ((GetFileAttributes(fileData.cFileName) & FILE_ATTRIBUTE_DIRECTORY) == 0)
			filesTotal++;

	return filesTotal;
}

// gets user audio file paths, returns the amount of supported audio files
int getUserAudioFiles(const char* path, OUT const char** fileList) {
	int i = 0;
	WIN32_FIND_DATA fileData;

	HANDLE hFile = FindFirstFile(TEXT(USER_AUDIO_FILES_PATH), &fileData);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	if ((GetFileAttributes(fileData.cFileName) & FILE_ATTRIBUTE_DIRECTORY) == 0 && isAllowedAudioFile(fileData.cFileName)) {
		strcpy_s(fileList[i], sizeof(fileList[i]), fileData.cFileName);
		i++;
	}

	while (FindNextFile(hFile, &fileData) != 0)
		if ((GetFileAttributes(fileData.cFileName) & FILE_ATTRIBUTE_DIRECTORY) == 0 && isAllowedAudioFile(fileData.cFileName)) {
			strcpy_s(fileList[i], sizeof(fileList[i]), fileData.cFileName);
			i++;
		}

	return i;
}

void playAudioThread(const char* filePath) {

}

// Toggles the audio playing thread, function not for multithread use
int togglePlayingAudio(const char* audioPath) {
	if (GetFileAttributes(audioPath) == INVALID_FILE_ATTRIBUTES)
		return PLAYER_COULDNT_FIND_AUDIO;

	if (InterlockedCompareExchange(&threadShouldBeRunning, FALSE, TRUE) == TRUE) // if thread had running state on, kill it to stop playing audio
		if (pthread_kill(soundPlayer, SIGINT) != 0) {
			InterlockedExchange(&threadShouldBeRunning, TRUE);
			return PLAYER_THREAD_FAILED_TO_KILL;
		}
		else
			return PLAYER_NO_ERROR;

	InterlockedExchange(&threadShouldBeRunning, TRUE);
	pthread_create(&soundPlayer, NULL, playAudioThread, audioPath);
	pthread_join(soundPlayer, NULL);

	return PLAYER_NO_ERROR;
}