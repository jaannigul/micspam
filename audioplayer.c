#include "audioplayer.h"
#include "audioswitcher.h"
#include "consts.h"
#include "deviceIDs.h"

#include <sndfile.h>
#include <samplerate.h>
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
		strcpy_s(fileList[i], MAX_PATH, fileData.cFileName);
		i++;
	}

	while (FindNextFile(hFile, &fileData) != 0)
		if ((GetFileAttributes(fileData.cFileName) & FILE_ATTRIBUTE_DIRECTORY) == 0 && isAllowedAudioFile(fileData.cFileName)) {
			strcpy_s(fileList[i], MAX_PATH, fileData.cFileName);
			i++;
		}

	return i;
}

void playAudioThread(const char* filePath) {
	SNDFILE* file;
	SF_INFO info;
	float* audioDataBuf = 0;
	float* tempAudioDataBuf = 0;

	// read the entire sound file to memory
	file = sf_open(filePath, SFM_READ, &info);
	if (!file) {
		fprintf(stderr, "Audio player thread could not open audio file: %s\n", filePath);
		goto cleanup;
	}

	audioDataBuf = calloc(info.channels * info.frames, sizeof(float));
	sf_read_float(file, audioDataBuf, info.channels*info.frames);
	if (cancellationRequest == TRUE) goto cleanup;

	// convert sample rate to something acceptable, if needed
	SRC_DATA conversionData = { 0 };
	if (info.samplerate != realMicSampleRate) {
		conversionData.src_ratio = realMicSampleRate / info.samplerate;
		tempAudioDataBuf = calloc((size_t)(conversionData.src_ratio * (double)info.channels * (double)info.frames), sizeof(float));
		conversionData.data_in = audioDataBuf;
		conversionData.data_out = tempAudioDataBuf;
		conversionData.input_frames = info.frames;
		conversionData.output_frames = (size_t)(conversionData.src_ratio * (double)info.frames);

		int res = src_simple(&conversionData, SRC_LINEAR, info.channels);
		if (res != 0)
			fprintf(stderr, "Audio player thread failed to convert sample rate, output audio can be wonky. Error: %s", src_strerror(res));
		else {
			free(audioDataBuf);
			audioDataBuf = calloc(conversionData.output_frames_gen * info.channels, sizeof(float));
			memcpy(audioDataBuf, tempAudioDataBuf, conversionData.output_frames_gen * info.channels * sizeof(float));
		}
	}
	else // we need to fill output_frames_gen with something since the following code uses it
		conversionData.output_frames_gen = info.frames * info.channels;

	if (cancellationRequest == TRUE) goto cleanup;

	int newFrameCount = conversionData.output_frames_gen * info.channels;
	free(tempAudioDataBuf);
	tempAudioDataBuf = calloc(newFrameCount / info.channels, sizeof(float));

	// simplify all channels down to one
	for (int i = 0; i < newFrameCount / info.channels; i++)
	{
		if (cancellationRequest == TRUE) goto cleanup;

		for (int j = 0; j < info.channels; j++)
			tempAudioDataBuf[i] += audioDataBuf[i * info.channels + j];
		tempAudioDataBuf[i] /= info.channels;
	}

	// send the data over, while periodically checking for cancel request
	int framesLeft = newFrameCount / info.channels;
	int framesCopied = 0;
	while (framesLeft > 0) {
		if (cancellationRequest == TRUE) goto cleanup;

		float* buf = calloc(BUFFER_FRAMES, sizeof(float));
		if (framesLeft >= BUFFER_FRAMES)
			memcpy(buf, tempAudioDataBuf + framesCopied * BUFFER_FRAMES, BUFFER_FRAMES*sizeof(float));
		else
			memcpy(buf, tempAudioDataBuf + framesCopied * BUFFER_FRAMES, framesLeft * sizeof(float));

		StsQueue.push(virtualMicPlaybackQueue, buf, MICSPAM_DATA_PRIORITY);

		framesCopied++;
		framesLeft -= BUFFER_FRAMES;
	}

cleanup:
	if(audioDataBuf) free(audioDataBuf);
	if(tempAudioDataBuf) free(tempAudioDataBuf);
	InterlockedExchange(&threadRunning, FALSE);
	InterlockedExchange(&cancellationRequest, FALSE);
	pthread_exit(NULL);
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