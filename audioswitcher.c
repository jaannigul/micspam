#include "audioswitcher.h"
#include "consts.h"
#include <string.h>
#include <stdio.h>
#include <Windows.h>
#include "deviceIDs.h"

StsHeader* virtualMicPlaybackQueue = NULL;
StsHeader* headphonesPlaybackQueue = NULL;

int realMicAndHeadphonesCallback(float* out, float* in, unsigned int nFrames,
    double stream_time, rtaudio_stream_status_t status,
    int* headphoneChannelCount) { // userdata: int* headphoneChannelCount

    float zero = 0.0f;
    float* forwardData = calloc(BUFFER_FRAMES, sizeof(float));
    float* playbackDataHeadphones = StsQueue.pop(headphonesPlaybackQueue);
    if (forwardData) {
        memcpy(forwardData, in, BUFFER_FRAMES*sizeof(float));
        StsQueue.push(virtualMicPlaybackQueue, forwardData, REAL_MIC_DATA_PRIORITY, TRUE);
    }
    if (playbackDataHeadphones) {
        int channels = *headphoneChannelCount;

        // 1 to N channel conversion, since our playback data is mono
        for (int i = 0; i < BUFFER_FRAMES; i++)
            for (int chnl = 0; chnl < channels; chnl++)
                *(out + i * channels + chnl) = *(playbackDataHeadphones + i);

        free(playbackDataHeadphones); // as we malloced it, we have to free this data

    }
    else memset(out, *(int*)&zero, *headphoneChannelCount * BUFFER_FRAMES * sizeof(float));


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
    headphonesPlaybackQueue = StsQueue.create();
    if (virtualMicPlaybackQueue == NULL) {
        printf("Failed to create audio playback queue to send audio from real mic to virtual mic.\n");
        return;
    }

    rtaudio_start_stream(realDeviceAudio);
    rtaudio_start_stream(virtualDeviceAudio);
    switchDefaultAudioInputDevice(VIRTUAL_AUDIO_DEVICE_INPUT_ID);


  
}

void closeStreamsAndCleanup(rtaudio_t realDeviceAudio, rtaudio_t virtualDeviceAudio) {
    StsQueue.destroy(virtualMicPlaybackQueue);

    switchDefaultAudioInputDevice(AUDIO_DEVICE_ID);
    rtaudio_close_stream(realDeviceAudio);
    rtaudio_close_stream(virtualDeviceAudio);
}