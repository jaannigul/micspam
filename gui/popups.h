#pragma once

#include <Windows.h>
#include <chrono>

enum PopupType {
	POPUP_TEST = 0,
	POPUP_TEXT,
	POPUP_SONG
};

typedef struct {
	PopupType type;
	void* userdata;
	int userdataCount;
} PopupData;

void handlePopupAnimation(HWND hWindow, std::chrono::steady_clock::time_point popupStartTime);
void displayCorrectPopup(HWND hWindow, PopupData data);