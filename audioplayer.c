#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <pthread.h>

typedef struct {
	const char* filename;
	int deviceIndex;
	float volume;
} PlayThreadInfo;



void* playAudio(void* arg) {
	PlayThreadInfo* info = (PlayThreadInfo*)arg;
	SNDFILE* infile;
	SF_INFO sfinfo;
	PaStream* stream;
	PaError err;
	PaStreamParameters outputParameters;
	printf("Thread filename: %s\n", info->filename);
	memset(&sfinfo, 0, sizeof(sfinfo));

	// Open the audio file
	infile = sf_open(info->filename, SFM_READ, &sfinfo);
	if (!infile) {
		fprintf(stderr, "Could not open audio file: %s\n", info->filename);
		return NULL;
	}

	// Set up output stream parameters to match the device settings
	outputParameters.device = info->deviceIndex;
	outputParameters.channelCount = sfinfo.channels;
	outputParameters.sampleFormat = paInt16;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	printf("Channel count: %d\n", sfinfo.channels);
	// Open an audio I/O stream.
	err = Pa_OpenStream(&stream, NULL, &outputParameters, sfinfo.samplerate, 256, paClipOff, NULL, NULL);
	if (err != paNoError) {
		fprintf(stderr, "Portaudio error when opening the stream: %s\n", Pa_GetErrorText(err));
		sf_close(infile);
		Pa_Terminate();
		return NULL;
	}

	// Start the audio stream
	err = Pa_StartStream(stream);
	if (err != paNoError) {
		fprintf(stderr, "PortAudio error when starting the stream: %s\n", Pa_GetErrorText(err));
		Pa_CloseStream(stream);
		sf_close(infile);
		Pa_Terminate();
		return NULL;
	}

	// Read and write samples
	short buffer[512];
	sf_count_t num_read;
	while ((num_read = sf_read_short(infile, buffer, 512)) > 0) {
		//volume
		for (int i = 0; i < num_read; i++) {
			buffer[i] = (short)(buffer[i] * info->volume);
		}
		Pa_WriteStream(stream, buffer, num_read);
	}

	// Close the stream and clean up
	Pa_StopStream(stream);
	Pa_CloseStream(stream);
	sf_close(infile);

	return NULL;
}


void listDevices() {
	int numDevices, i;
	const PaDeviceInfo* deviceInfo;
	Pa_Initialize();
	numDevices = Pa_GetDeviceCount();
	if (numDevices < 0) {
		printf("ERROR: Pa_CountDevices returned 0x%x\n", numDevices);
		return;
	}

	for (i = 0; i < numDevices; i++) {
		deviceInfo = Pa_GetDeviceInfo(i);
		printf("%d: %s\n", i, deviceInfo->name);
	}
	Pa_Terminate();
}


//Function for switching the audio input device

void switchAudioInputDevice(const char* targetDevice) {
	char command[1024];
	printf("Switching audio input device\n");
	sprintf_s(command, "powershell -ExecutionPolicy Bypass -File \"changeAudioDevice.ps1\" -DeviceName \"%s\"", targetDevice);
	printf("Executing command: %s\n", command);
	system(command);
}

void startPlayingAudio() {
	Pa_Initialize();
	listDevices();
	//playAudio("C:\\Users\\PC\\Desktop\\prog\\slam\\csgo\\villager.wav");
	const char* filename = "C:\\Users\\PC\\Desktop\\prog\\slam\\csgo\\Yuno Miles - Hong Kong (Official Video) (Prod.Vinxia).wav";
	int deviceIndexHeadphones = 4;
	int deviceIndexVAC = 5;
	float headphoneVolume = 0.05f;
	float vacVolume = 0.001f;
	pthread_t thread1, thread2;
	PlayThreadInfo* info1 = malloc(sizeof(PlayThreadInfo));
	PlayThreadInfo* info2 = malloc(sizeof(PlayThreadInfo));
	*info1 = (PlayThreadInfo){ filename, deviceIndexHeadphones, headphoneVolume };
	*info2 = (PlayThreadInfo){ filename, deviceIndexVAC, vacVolume };
	printf("%s\n", info1->filename);
	pthread_create(&thread1, NULL, playAudio, info1);
	pthread_create(&thread2, NULL, playAudio, info2);

	//Wait for both threads to finish
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	free(info1);
	free(info2);

	Pa_Terminate();
}

int main() {
	
	switchAudioInputDevice("micspam");
	printf("aaa\n");
	startPlayingAudio();
	switchAudioInputDevice("Microphone");
	return 0;
}