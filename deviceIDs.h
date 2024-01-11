#pragma once
extern char* VIRTUAL_AUDIO_DEVICE_ID;
extern char* AUDIO_DEVICE_ID;
extern char* HEADPHONES_ID;

char* queryDeviceIDbyIndex(int deviceIndex);
void switchDefaultAudioInputDevice(const char* targetDeviceID);
