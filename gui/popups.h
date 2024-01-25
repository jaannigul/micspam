#pragma once

#include <Windows.h>
#include <chrono>

enum PopupType {
	POPUP_TEXT,
	POPUP_SONGS
};

typedef struct {
	PopupType type;
	void* userdata;
	int userdataCount;
	int userdataIndex;
} PopupData;

bool handlePopupAnimation(HWND hWindow, std::chrono::steady_clock::time_point popupStartTime, bool isPopupVisible);
void displayCorrectPopup(HWND hWindow, PopupData data, int popupX, int popupY);