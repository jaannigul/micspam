#pragma once
#include <stddef.h>
extern char VIRTUAL_AUDIO_DEVICE_ID[256];
extern char AUDIO_DEVICE_ID[256];
extern char HEADPHONES_ID[256];

void queryDeviceIDbyName(char* deviceName, char* deviceID, size_t size);
void switchDefaultAudioInputDevice(const char* targetDeviceID);
