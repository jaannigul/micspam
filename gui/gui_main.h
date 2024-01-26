#pragma once

#include "../utils/queue/sts_queue.h"
#include "popups.h"

EXTERN_C_START

extern StsHeader* popupTypesQueue;

int guiTestEntryPoint();
void sendPopupNotification(enum PopupType type, void* userdata, int userdataCount, int userdataIndex, int textFlags);

EXTERN_C_END