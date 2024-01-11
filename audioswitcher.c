#include "audioswitcher.h"
#include "consts.h"
#include "utils/queue/sts_queue.h"

#include <stdio.h>
#include <Windows.h>
#include "deviceIDs.h"

StsHeader* virtualMicPlaybackQueue = NULL;

int realMicAndHeadphonesCallback(INT16* out, INT16* in, unsigned int nFrames,
    double stream_time, rtaudio_stream_status_t status,
    void* userdata) {

    INT16* forwardData = calloc(BUFFER_FRAMES, sizeof(INT16));
    if (forwardData) {
        memcpy(forwardData, in, BUFFER_FRAMES*sizeof(INT16));
        StsQueue.push(virtualMicPlaybackQueue, forwardData, REAL_MIC_DATA_PRIORITY);
    }

    return 0;
}

int virtualMicCallback(void* out, void* in, unsigned int nFrames,
    double stream_time, rtaudio_stream_status_t status,
    void* userdata) {

    INT16* playbackData = StsQueue.pop(virtualMicPlaybackQueue);
    if (playbackData)
        memcpy(out, playbackData, BUFFER_FRAMES * sizeof(INT16));

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
    switchDefaultAudioInputDevice(VIRTUAL_AUDIO_DEVICE_ID);

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