#include "audioplayer.h"
#include "audioswitcher.h"
#include "consts.h"

#include <sndfile.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>

pthread_t soundPlayer;
volatile _Bool threadRunning = FALSE; // shitty workaround for stdatomic.h to check if our audio player thread is running
volatile _Bool cancellationRequest = FALSE;

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
	SNDFILE* file;
	SF_INFO info;
	float* audioDataBuf;

	// read the entire sound file to memory
	file = sf_open(filePath, SFM_READ, &info);
	if (!file) {
		fprintf(stderr, "Audio player thread could not open audio file: %s\n", filePath);
		return;
	}

	// TODO: fix different sample rate wav being sent to the microphone as input
	// this could cause distortion
	sf_count_t num_read = 0;

	do {
		int sendBufIdx = 0; 
		float* sendBuf = calloc(BUFFER_FRAMES, sizeof(float));
		for (int repeat = 0; repeat < info.channels; repeat++) { // required for us to fully fill the chunk after 
			num_read = sf_read_short(file, audioDataBuf, BUFFER_FRAMES);
			if (num_read <= 0) break;

			for (int i = 0; i < num_read / info.channels - info.channels; i++) {
				//for (int j = 0; j < info.channels; j++)
					sendBuf[sendBufIdx] += audioDataBuf[i * info.channels+1];
				//sendBuf[sendBufIdx] /= info.channels;
				sendBufIdx++;
			}
		}

		// add the data to mic queue
		// TODO: add this audio to headphone playback queue too
		StsQueue.push(virtualMicPlaybackQueue, sendBuf, MICSPAM_DATA_PRIORITY);
	} while (num_read > 0);

cleanup:
	free(audioDataBuf);
	InterlockedExchange(&threadRunning, FALSE);
	InterlockedExchange(&cancellationRequest, FALSE);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
}

// Toggles the audio playing thread, function not for multithread use
int togglePlayingAudio(const char* audioPath) {
	if (GetFileAttributes(audioPath) == INVALID_FILE_ATTRIBUTES)
		return PLAYER_COULDNT_FIND_AUDIO;

	if (InterlockedCompareExchange(&threadRunning, TRUE, TRUE) == TRUE) { // if thread had running state on, kill it to stop playing audio
		InterlockedExchange(&cancellationRequest, TRUE);

		struct timespec ts;
		if(timespec_get(&ts, TIME_UTC) == -1)
			return PLAYER_THREAD_FAILED_TO_KILL;
		ts.tv_sec += 1;

		if (pthread_timedjoin_np(soundPlayer, NULL, &ts) != 0)
			return PLAYER_THREAD_FAILED_TO_KILL;
		else {
			StsQueue.freeAllValues(virtualMicPlaybackQueue); // free all malloced values to avoid a huge memory leak
			StsQueue.removeAll(virtualMicPlaybackQueue);

			return PLAYER_NO_ERROR;
		}
	}

	InterlockedExchange(&threadRunning, TRUE);
	pthread_create(&soundPlayer, NULL, playAudioThread, audioPath);
	pthread_join(soundPlayer, NULL);

	return PLAYER_NO_ERROR;
}