#include "popups.h"
#include "popup_settings.h"
#include "windowutil.h"

#include <Windows.h>
#include <chrono>

// Handles the showing and transparency part for a popup window
// Returns if popup is still active
void handlePopupAnimation(HWND hWindow, std::chrono::steady_clock::time_point popupStartTime) {
	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

	int64_t durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - popupStartTime).count();

	/*if (durationMs > POPUP_SHOW_TIME_MS + POPUP_FADE_TIME_MS) {
		hideWindow(hWindow);
	}
	else if (durationMs > POPUP_SHOW_TIME_MS) {
		double timeLeftUntilInvis = POPUP_FADE_TIME_MS - (durationMs - POPUP_SHOW_TIME_MS);
		int alpha = 255 * (timeLeftUntilInvis / POPUP_FADE_TIME_MS);
		setWindowTransparency(hWindow, alpha);
	}*/
}

void drawTestPopup(HWND hWindow, PopupData data) {
	PAINTSTRUCT ps2;
	HDC dc = BeginPaint(hWindow, &ps2);

	RECT lowerWhiteBar = {
		0,
		POPUP_HEIGHT - WHITE_BAR_HEIGHT,
		POPUP_WIDTH,
		POPUP_HEIGHT
	};
	FillRect(dc, &lowerWhiteBar, CreateSolidBrush(RGB(255, 255, 255)));

	EndPaint(hWindow, &ps2);
}

void drawTextPopup(HWND hWindow, PopupData data) {
	char* text = static_cast<char*>(data.userdata);

	HFONT arial = CreateFont(24, 0, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_SWISS, "Arial");

	PAINTSTRUCT ps2;
	HDC dc = BeginPaint(hWindow, &ps2);

	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, RGB(255, 255, 255));

	RECT lowerWhiteBar = {
		0, POPUP_HEIGHT - WHITE_BAR_HEIGHT,
		POPUP_WIDTH,
		POPUP_HEIGHT
	};
	FillRect(dc, &lowerWhiteBar, CreateSolidBrush(RGB(255, 255, 255)));

	// format the text
	RECT textArea = {
		0, 0,
		POPUP_WIDTH, POPUP_HEIGHT - WHITE_BAR_HEIGHT
	};
	SelectObject(dc, arial);
	DrawText(dc, text, strlen(text), &textArea, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	DeleteObject(arial);
	EndPaint(hWindow, &ps2);
}


void displayCorrectPopup(HWND hWindow, PopupData data) {
	setWindowPosAndSize(hWindow, 0, 0, POPUP_WIDTH, POPUP_HEIGHT);

	switch (data.type) {
	case POPUP_TEST:
		drawTestPopup(hWindow, data);
		break;
	case POPUP_TEXT:
		drawTextPopup(hWindow, data);
	}
}