#include "audioswitcher.h"
#include "consts.h"

#include <stdio.h>
#include <Windows.h>
#include "deviceIDs.h"

StsHeader* virtualMicPlaybackQueue = NULL;

int realMicAndHeadphonesCallback(float* out, float* in, unsigned int nFrames,
    double stream_time, rtaudio_stream_status_t status,
    void* userdata) {

    float* forwardData = calloc(BUFFER_FRAMES, sizeof(float));
    if (forwardData) {
        memcpy(forwardData, in, BUFFER_FRAMES*sizeof(float));
        StsQueue.push(virtualMicPlaybackQueue, forwardData, REAL_MIC_DATA_PRIORITY);
    }

    return 0;
}

int virtualMicCallback(float* out, float* in, unsigned int nFrames,
    double stream_time, rtaudio_stream_status_t status,
    void* userdata) {

    float* playbackData = StsQueue.pop(virtualMicPlaybackQueue);
    if (playbackData) {
        memcpy(out, playbackData, BUFFER_FRAMES * sizeof(float));
        free(playbackData); // as we malloced it, we have to free this data
    }

    return 0;
}

void startSwitchingAudio(rtaudio_t realDeviceAudio, rtaudio_t virtualDeviceAudio) {
    virtualMicPlaybackQueue = StsQueue.create();
    if (virtualMicPlaybackQueue == NULL) {
        printf("Failed to create audio playback queue to send audio from real mic to virtual mic.\n");
        return;
    }

    rtaudio_start_stream(realDeviceAudio);
    rtaudio_start_stream(virtualDeviceAudio);
    switchDefaultAudioInputDevice(VIRTUAL_AUDIO_DEVICE_INPUT_ID);
    togglePlayingAudio("./audiosamples/skrillex.wav");

    // TODO: make this the main thread for looking at keypresses (quitting app, playing sound)
    while (1) {
        //if(GetAsyncKeyState()) break;
        Sleep(1);
    }

    StsQueue.destroy(virtualMicPlaybackQueue);

    switchDefaultAudioInputDevice(AUDIO_DEVICE_ID);
    rtaudio_close_stream(realDeviceAudio);
    rtaudio_close_stream(virtualDeviceAudio);
}