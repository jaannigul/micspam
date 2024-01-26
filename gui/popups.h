#pragma once

#include <Windows.h>

enum PopupType {
	POPUP_KEEP_AWAKE,
	POPUP_TEXT,
	POPUP_SONGS
};

typedef struct {
	enum PopupType type;
	void* userdata;
	int userdataCount;
	int userdataIndex;
	int textFlags;
} PopupData;

#ifdef __cplusplus
#include <chrono>

bool handlePopupAnimation(HWND hWindow, std::chrono::steady_clock::time_point popupStartTime, bool isPopupVisible);
void displayCorrectPopup(HWND hWindow, PopupData data);

#endif