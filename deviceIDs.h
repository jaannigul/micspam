#pragma once
#include <stddef.h>
extern char VIRTUAL_AUDIO_DEVICE_OUTPUT_ID[256]; //stream the audio here
extern char VIRTUAL_AUDIO_DEVICE_INPUT_ID[256]; //set this as default recording device
extern char AUDIO_DEVICE_ID[256];
extern char HEADPHONES_ID[256];

extern double realMicSampleRate;

void queryDeviceIDbyName(char* deviceName, char* deviceID, size_t size);
void switchDefaultAudioInputDevice(const char* targetDeviceID);
