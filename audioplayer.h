#pragma once

#include <pthread.h>

extern pthread_t soundPlayer;
extern pthread_mutex_t threadCreationLock;

_Bool setupAudioPlayer();