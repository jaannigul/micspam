#include "popups.h"
#include "popup_settings.h"
#include "windowutil.h"

#include <Windows.h>
#include <chrono>

// Handles the showing and transparency part for a popup window
void handlePopupAnimation(HWND hWindow, std::chrono::steady_clock::time_point popupStartTime) {
	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

	int64_t durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - popupStartTime).count();

	if (durationMs > POPUP_SHOW_TIME_MS + POPUP_FADE_TIME_MS)
		hideWindow(hWindow);
	else if (durationMs > POPUP_SHOW_TIME_MS) {
		double timeLeftUntilInvis = POPUP_FADE_TIME_MS - (durationMs - POPUP_SHOW_TIME_MS);
		int alpha = 255 * (timeLeftUntilInvis / POPUP_FADE_TIME_MS);
		setWindowTransparency(hWindow, alpha);
	}
	else
		setWindowTransparency(hWindow, 255);
}

void drawTestPopup(HWND hWindow, PopupData data) {

}

void displayCorrectPopup(HWND hWindow, PopupData data) {
	switch (data.type) {
	case POPUP_TEST:
		drawTestPopup(hWindow, data);
	}
}