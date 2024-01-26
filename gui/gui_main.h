#pragma once

#include "../utils/queue/sts_queue.h"
#include "popups.h"

extern StsHeader* popupTypesQueue;

EXTERN_C_START

int guiTestEntryPoint();
void sendPopupNotification(enum PopupType type, void* userdata, int userdataCount, int userdataIndex, int textFlags);

EXTERN_C_END