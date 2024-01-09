#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <pthread.h>

char CURRENT_MUSIC_FILE[_MAX_PATH];
PaStream* DEFAULT_MIC_STREAM = 0;
PaStream* DEFAULT_HEADPHONES_STREAM = 0;

typedef struct {
    const char* filename;
    int inputDeviceIndex; // read sound from here
    int outputDeviceIndex; // output sound to here
    float volume;
} PlayThreadInfo;


void initializeAudio() {
	Pa_Initialize();
}

//Function for listening for mic / micspam input and switching the audio input device 
void listenToMicAudioCallback(const void* input,
    void* output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) {

}

// Function for continuously playing audio through the virtual microphone based on buffered input
void playAudioThroughVirtualMic(PlayThreadInfo* virtualMic) {

}

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
    outputParameters.device = info->outputDeviceIndex;
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
    numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        printf("ERROR: Pa_CountDevices returned 0x%x\n", numDevices);
        return;
    }

    for (i = 0; i < numDevices; i++) {
        deviceInfo = Pa_GetDeviceInfo(i);
        printf("%d: %s, samplerate: %f, input channels: %d, output channels: %d\n", i, deviceInfo->name, deviceInfo->defaultSampleRate, deviceInfo->maxInputChannels, deviceInfo->maxOutputChannels);
    }
}

// Let user choose the mic he wants to take his own sound from and headphones for micspam playback
_Bool askForMicAndHeadphones() {
    char micSelectedId[8] = { 0 }, headphonesSelectedId[8] = { 0 };
    PaError err;

    listDevices();

    printf("Choose your microphone device's number from the list: ");
    fgets(micSelectedId, sizeof(micSelectedId), stdin);
    printf("Choose your headphone device's number from the list: ");
    fgets(headphonesSelectedId, sizeof(headphonesSelectedId), stdin);
    int micId = atoi(micSelectedId);
    int headphonesId = atoi(headphonesSelectedId);

    PaDeviceInfo* micInfo = Pa_GetDeviceInfo(micId);
    PaStreamParameters micStreamParams = { 0 }, headphoneStreamParams = { 0 };
    micStreamParams.device = micId;
    micStreamParams.channelCount = min(micInfo->maxInputChannels, 2);
    micStreamParams.sampleFormat = paInt16;
    micStreamParams.suggestedLatency = micInfo->defaultLowOutputLatency;

    if((err = Pa_OpenStream(&DEFAULT_MIC_STREAM, &micStreamParams, NULL, micInfo->defaultSampleRate, 256, paClipOff, &listenToMicAudioCallback, NULL)) != paNoError) {
        fprintf(stdout, "Portaudio error when opening microphone stream: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        return 0;
    }

    PaDeviceInfo* headphonesInfo = Pa_GetDeviceInfo(headphonesId);
    headphoneStreamParams.device = headphonesId;
    headphoneStreamParams.channelCount = min(headphonesInfo->maxInputChannels, 2);
    headphoneStreamParams.sampleFormat = paInt16;
    headphoneStreamParams.suggestedLatency = headphonesInfo->defaultLowOutputLatency;

    if ((err = Pa_OpenStream(&DEFAULT_HEADPHONES_STREAM, NULL, &headphoneStreamParams, headphonesInfo->defaultSampleRate, 256, paClipOff, NULL, NULL)) != paNoError) {
        fprintf(stdout, "Portaudio error when opening headphone stream: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        return 0;
    }

    return 1;
}

int main() {
    Pa_Initialize();

    if (!askForMicAndHeadphones()) return;

    const char* filename = "C:\\Users\\PC\\Desktop\\prog\\slam\\csgo\\Yuno Miles - Hong Kong (Official Video) (Prod.Vinxia).wav";
    int deviceIndexHeadphones = 4;
    int deviceIndexVAC = 5;
    float headphoneVolume = 0.05f; 
    float vacVolume = 0.001f; 
    pthread_t thread1;
    PlayThreadInfo* info1 = malloc(sizeof(PlayThreadInfo));
    PlayThreadInfo* info2 = malloc(sizeof(PlayThreadInfo));
    *info1 = (PlayThreadInfo){ filename, deviceIndexHeadphones, headphoneVolume };
    *info2 = (PlayThreadInfo){ filename, deviceIndexVAC, vacVolume };
    printf("%s\n", info1->filename);

    pthread_create(&thread1, NULL, playAudioThroughVirtualMic, info2);

    //Wait for both threads to finish
    pthread_join(thread1, NULL);

    free(info1);
    free(info2);

    Pa_Terminate();

    return 0;
}