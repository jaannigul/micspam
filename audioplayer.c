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

// copied from https://github.com/libsndfile/libsamplerate/blob/master/examples/varispeed-play.c

typedef struct
{
	int			magic;
	SNDFILE* sndfile;
	SF_INFO 	sfinfo;

	float		buffer[BUFFER_FRAMES];
} SNDFILE_CB_DATA;

#define ARRAY_LEN(x)	((int) (sizeof (x) / sizeof ((x) [0])))

// what the fuck does this do
static long
src_input_callback(void* cb_data, float** audio)
{
	SNDFILE_CB_DATA* data = (SNDFILE_CB_DATA*)cb_data;
	const int input_frames = ARRAY_LEN(data->buffer) / data->sfinfo.channels;
	int		read_frames;

	for (read_frames = 0; read_frames < input_frames; )
	{
		sf_count_t position;

		read_frames += (int)sf_readf_float(data->sndfile, data->buffer + read_frames * data->sfinfo.channels, input_frames - read_frames);

		position = sf_seek(data->sndfile, 0, SEEK_CUR);

		if (position < 0 || position == data->sfinfo.frames)
			sf_seek(data->sndfile, 0, SEEK_SET);
	};

	*audio = &(data->buffer[0]);

	return input_frames;
}

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

	HANDLE hFile = FindFirstFile(TEXT(USER_AUDIO_FILES_PATH_WILDCARD), &fileData);
	if (hFile == INVALID_HANDLE_VALUE)
		return filesTotal;

	if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		filesTotal++;

	while (FindNextFile(hFile, &fileData) != 0) {
		if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			filesTotal++;
	}

	return filesTotal;
}

// gets user audio file paths, returns the amount of supported audio files
int getUserAudioFiles(const char* path, OUT const char** fileList) {
	int i = 0;
	WIN32_FIND_DATA fileData;

	HANDLE hFile = FindFirstFile(TEXT(USER_AUDIO_FILES_PATH_WILDCARD), &fileData);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 && isAllowedAudioFile(fileData.cFileName)) {
		strcpy_s(fileList[i], MAX_PATH, fileData.cFileName);
		i++;
	}

	while (FindNextFile(hFile, &fileData) != 0)
		if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 && isAllowedAudioFile(fileData.cFileName)) {
			sprintf_s(fileList[i], MAX_PATH, "%s\\%s", USER_AUDIO_FILES_PATH, fileData.cFileName);
			i++;
		}

	return i;
}

char** allocateFileList(int numFiles) {
	char** fileList = malloc(numFiles * sizeof(char*));
	for (int i = 0; i < numFiles; i++) {
		fileList[i] = malloc(MAX_PATH * sizeof(char));
	}
	return fileList;
}

//play the loaded audio file
void playAudioThread(const char* filePath) {
	printf("Playing audio\n");
	printf("path : %s\n", filePath);

	// read the entire sound file to memory
	SF_INFO info;
	SNDFILE* file = sf_open(filePath, SFM_READ, &info);
	if (!file) {
		fprintf(stderr, "Audio player thread could not open audio file: %s\n", filePath);
		return;
	}

	SRC_STATE* converter = src_new(SRC_LINEAR, info.channels, NULL);
	if (!converter) {
		fprintf(stderr, "Audio player thread failed to open sample rate converter.\n");
		return;
	}

	float* audioDataBuf = calloc(info.channels * info.frames, sizeof(float));
	sf_read_float(file, audioDataBuf, info.channels * info.frames);

	SRC_DATA conversionData = { 0 };
	int inputFramesLeft = info.frames;
	conversionData.src_ratio = realMicSampleRate / info.samplerate;
	float* tempAudioDataBuf = calloc((size_t)ceil(conversionData.src_ratio * (double)info.channels * BUFFER_FRAMES), sizeof(float));
	conversionData.data_in = audioDataBuf;
	conversionData.data_out = tempAudioDataBuf;
	conversionData.input_frames = info.frames;
	conversionData.output_frames = BUFFER_FRAMES;

	if (cancellationRequest == TRUE) goto cleanup;

	// convert frames one by one, send them over to virtual mic and playback
	while (inputFramesLeft > 0) {
		if (cancellationRequest == TRUE) goto cleanup;

		int res = src_process(converter, &conversionData); // TODO: check the error in future

		conversionData.data_in += conversionData.input_frames_used * info.channels; // keep moving the data pointer by the amount of frames used
		inputFramesLeft -= conversionData.input_frames_used;
		conversionData.input_frames = inputFramesLeft;

		int monoFrames = min(conversionData.output_frames_gen, BUFFER_FRAMES);
		float* framesToSendSoon = calloc(monoFrames, sizeof(float));

		// simplify all channels down to one
		for (int i = 0; i < monoFrames; i++)
		{
			for (int j = 0; j < info.channels; j++)
				framesToSendSoon[i] += tempAudioDataBuf[i * info.channels + j];
			framesToSendSoon[i] /= info.channels;
			framesToSendSoon[i] *= soundVolume;

		}

		float* buf = calloc(BUFFER_FRAMES, sizeof(float));
		float* buf2 = calloc(BUFFER_FRAMES, sizeof(float));
		if (!buf || !buf2) {
			free(framesToSendSoon);
			continue;
		}

		memcpy(buf, framesToSendSoon, monoFrames * sizeof(float));
		memcpy(buf2, buf, monoFrames * sizeof(float));

		StsQueue.push(virtualMicPlaybackQueue, buf, MICSPAM_DATA_PRIORITY, TRUE);
		StsQueue.push(headphonesPlaybackQueue, buf2, MICSPAM_DATA_PRIORITY, TRUE);

		free(framesToSendSoon);
	}

	while (!StsQueue.isEmpty(headphonesPlaybackQueue)) { // loop required to make abruptly stopping audio work, as the toggleaudioplayer func looks if the thread is running, not if mic queue has micspam data in it
		if (cancellationRequest == TRUE) goto cleanup;
		Sleep(1);
	}

cleanup:
	if (file) sf_close(file);
	if (converter) src_delete(converter);
	if(audioDataBuf) free(audioDataBuf);
	if(tempAudioDataBuf) free(tempAudioDataBuf);
	InterlockedExchange(&threadRunning, FALSE);
	InterlockedExchange(&cancellationRequest, FALSE);
	pthread_exit(NULL);
}

const char* getFileName(const char* filePath) {
	const char* lastBackslash = strrchr(filePath, '\\');
	return lastBackslash ? lastBackslash + 1 : filePath;
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
			StsQueue.removeAll(virtualMicPlaybackQueue);
			StsQueue.removeAll(headphonesPlaybackQueue);

			return PLAYER_THREAD_KILLED;
		}
	}

	InterlockedExchange(&threadRunning, TRUE);
	pthread_create(&soundPlayer, NULL, playAudioThread, audioPath);
	return PLAYER_NO_ERROR;
}