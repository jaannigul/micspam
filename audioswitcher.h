#pragma once
#include <rtaudio/rtaudio_c.h>

int virtualMicCallback(void* out, void* in, unsigned int nFrames,
    double stream_time, rtaudio_stream_status_t status,
    void* userdata);

int realMicAndHeadphonesCallback(int* out, int* in, unsigned int nFrames,
    double stream_time, rtaudio_stream_status_t status,
    void* userdata);

void startSwitchingAudio(rtaudio_t realDeviceAudio, rtaudio_t virtualDeviceAudio);