#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>

void initializeAudio() {
	Pa_Initialize();
}

void playAudio(const char* filename) {
    SNDFILE* infile;
    SF_INFO sfinfo;
    PaStream* stream;
    PaError err;
    PaStreamParameters outputParameters;

    // Open the audio file
    infile = sf_open(filename, SFM_READ, &sfinfo);
    if (!infile) {
        fprintf(stderr, "Could not open audio file: %s\n", filename);
        return;
    }

    // Initialize PortAudio
    Pa_Initialize();

    // Set up output stream parameters
    outputParameters.device = 5; // Device index for Virtual Audio Cable input
    outputParameters.channelCount = sfinfo.channels; // Set the same number of channels as in the audio file
    //outputParameters.channelCount = 2; // Set the same number of channels as in the audio file
    outputParameters.sampleFormat = paInt16; // Assuming the file is in float format paFloat32
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    

    // Open an audio I/O stream.
    err = Pa_OpenStream(&stream, NULL, &outputParameters, sfinfo.samplerate, 256, paClipOff, NULL, NULL);
    if (err != paNoError) {
        fprintf(stderr, "Portaudio error: %s, channelCount: %d\n", Pa_GetErrorText(err), sfinfo.channels);
        sf_close(infile);
        Pa_Terminate();
        return;
    }
    // Start the audio stream
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(stream);
        sf_close(infile);
        Pa_Terminate();
        return;
    }

    // Read and write samples
 
    printf("Sample rate: %d\nChannels: %d", sfinfo.samplerate,sfinfo.channels);
    if (sfinfo.channels>1)
    {
        short monoBuffer[512];
        short stereoBuffer[1024];
        sf_count_t num_read;
        memset(stereoBuffer, 0, sizeof(stereoBuffer));
        while ((num_read = sf_read_short(infile, monoBuffer, 512)) > 0) {
            for (int i = 0; i < num_read; ++i) {
                stereoBuffer[2 * i] = monoBuffer[i];     // Left channel
                stereoBuffer[2 * i + 1] = monoBuffer[i]; // Right channel
            }
            Pa_WriteStream(stream, stereoBuffer, num_read);
        }
    }
    else {
        short buffer[512];
        sf_count_t num_read;
        while ((num_read = sf_read_short(infile, buffer, 512)) > 0) {
            Pa_WriteStream(stream, buffer, num_read);
        }
    }
    
    // Close the stream and clean up
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    sf_close(infile);
    Pa_Terminate();
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
void switchAudioInputDevice() {

}

int main() {
    listDevices();
    playAudio("C:\\Users\\PC\\Desktop\\prog\\slam\\csgo\\Skrillex - First Of The Year (Equinox) [Official Audio].wav");
    return 0;
}