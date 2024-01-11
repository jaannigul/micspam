#pragma once

typedef struct {
	const char* filename;
	int deviceIndex;
	float volume;
} PlayThreadInfo;
void* playAudio(void*);
void listDevices();
void switchAudioInputDevice();
void startPlayingAudio();
