#pragma once

#include <Windows.h>
#include "../utils/queue/sts_queue.h"

enum PopupTypes : int {
	POPUP_TEST = 0,
	POPUP_VOLUME
};

extern StsHeader* popupTypesQueue;

EXTERN_C_START

int guiTestEntryPoint();

EXTERN_C_END