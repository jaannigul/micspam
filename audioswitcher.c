#include "audioswitcher.h"
#include "globals.h"

#include <stdio.h>
#include <Windows.h>

HANDLE virtualMicDataMutex = INVALID_HANDLE_VALUE;
INT16 virtualMicBuf[BUFFER_FRAMES];

_Bool shouldForwardMicData = 1;

int realMicAndHeadphonesCallback(INT16* out, INT16* in, unsigned int nFrames,
    double stream_time, rtaudio_stream_status_t status,
    void* userdata) {

    if(shouldForwardMicData && WaitForSingleObject(virtualMicDataMutex, 20) == WAIT_OBJECT_0)
        memcpy(virtualMicBuf, in, BUFFER_FRAMES);
    ReleaseMutex(virtualMicDataMutex);

    return 0;
}

int virtualMicCallback(void* out, void* in, unsigned int nFrames,
    double stream_time, rtaudio_stream_status_t status,
    void* userdata) {

    if (shouldForwardMicData) {
        if (WaitForSingleObject(virtualMicDataMutex, 20) == WAIT_OBJECT_0)
            memcpy(out, virtualMicBuf, BUFFER_FRAMES);
        ReleaseMutex(virtualMicDataMutex);
    }

    return 0;
}

void startSwitchingAudio(rtaudio_t realDeviceAudio, rtaudio_t virtualDeviceAudio) {
    virtualMicDataMutex = CreateMutex(NULL, TRUE, TEXT("micToVmic"));
    if (virtualMicDataMutex == INVALID_HANDLE_VALUE) {
        printf("Failed to create a mutex for necessary sound data transfer between mic and virtual mic.\n");
        return;
    }

    rtaudio_start_stream(realDeviceAudio);
    rtaudio_start_stream(virtualDeviceAudio);

    // TODO: make this the main thread for looking at keypresses (quitting app, playing sound)
    while (1) {
        //if(GetAsyncKeyState()) break;
        Sleep(1);
    }

    rtaudio_close_stream(realDeviceAudio);
    rtaudio_close_stream(virtualDeviceAudio);
}