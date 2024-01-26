#include "popups.h"
#include "popup_settings.h"
#include "windowutil.h"
#include "../consts.h"

#include <Windows.h>
#include <chrono>
#include <string>
#include <format>
#include <iostream>

// Handles the showing and transparency part for a popup window
// Returns if popup is still active
bool handlePopupAnimation(HWND hWindow, std::chrono::steady_clock::time_point popupStartTime, bool isPopupVisible) {
	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

	int64_t durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - popupStartTime).count();

	if (durationMs > POPUP_SHOW_TIME_MS + POPUP_FADE_TIME_MS) {
		hideWindow(hWindow);
		return false;
	}
	else if (durationMs > POPUP_SHOW_TIME_MS) {
		double timeLeftUntilInvis = POPUP_FADE_TIME_MS - (durationMs - POPUP_SHOW_TIME_MS);
		int alpha = 255 * (timeLeftUntilInvis / POPUP_FADE_TIME_MS);
		setWindowTransparency(hWindow, alpha);
		return true;
	}

	return true;
}

void drawMultipleSongsPopup(HWND hWindow, PopupData data) {
	HFONT arial = CreateFont(SMALL_FONT_HEIGHT, 0, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_SWISS, "Arial");
	HFONT arialBold = CreateFont(LARGE_FONT_HEIGHT, 0, 0, 0, FW_BOLD, false, false, false, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_SWISS, "Arial");

	PAINTSTRUCT ps2;
	HDC dc = BeginPaint(hWindow, &ps2);

	// text color and background for text
	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, RGB(255, 255, 255));

	// format the song names into strings
	int filePrefixLen = 0;//strlen(USER_AUDIO_FILES_PATH) + 1;
	char** start = static_cast<char**>(data.userdata);
	std::string prevSong, currentSong, nextSong;
	currentSong = std::format("Curr. {}", start[data.userdataIndex] + filePrefixLen);
	if (data.userdataIndex - 1 < 0)
		prevSong = std::format("Prev. {}", start[data.userdataCount - 1] + filePrefixLen);
	else 
		prevSong = std::format("Prev. {}", start[data.userdataIndex - 1] + filePrefixLen);

	if(data.userdataIndex+1 >= data.userdataCount)
		nextSong = std::format("Next. {}", start[0] + filePrefixLen);
	else
		nextSong = std::format("Next. {}", start[data.userdataIndex + 1] + filePrefixLen);

	RECT lowerWhiteBar = {
		0,
		POPUP_HEIGHT - WHITE_BAR_HEIGHT,
		POPUP_WIDTH,
		POPUP_HEIGHT
	};
	FillRect(dc, &lowerWhiteBar, CreateSolidBrush(RGB(255, 255, 255)));

	// rects for each text
	int yStart = (POPUP_HEIGHT - WHITE_BAR_HEIGHT - 2* SMALL_FONT_HEIGHT - 1*LARGE_FONT_HEIGHT - 3*TEXT_MARGIN_TOP) / 2; // get the amount of y we need to start from so that all 3 lines of text is centered, or close to being centered

	RECT prevTextArea = {
		5, yStart,
		POPUP_WIDTH, yStart + SMALL_FONT_HEIGHT
	};
	yStart += SMALL_FONT_HEIGHT + TEXT_MARGIN_TOP;
	RECT currTextArea = {
		5, yStart,
		POPUP_WIDTH, yStart + LARGE_FONT_HEIGHT
	};
	yStart += LARGE_FONT_HEIGHT + TEXT_MARGIN_TOP;
	RECT nextTextArea = {
		5, yStart,
		POPUP_WIDTH, yStart + SMALL_FONT_HEIGHT
	};

	SelectObject(dc, arial);
	DrawText(dc, prevSong.c_str(), prevSong.length(), &prevTextArea, DT_SINGLELINE | DT_WORD_ELLIPSIS);

	SelectObject(dc, arialBold);
	DrawText(dc, currentSong.c_str(), currentSong.length(), &currTextArea, DT_SINGLELINE | DT_WORD_ELLIPSIS);

	SelectObject(dc, arial);
	DrawText(dc, nextSong.c_str(), nextSong.length(), &nextTextArea, DT_SINGLELINE | DT_WORD_ELLIPSIS);

	DeleteObject(arial);
	DeleteObject(arialBold);
	EndPaint(hWindow, &ps2);
}

void drawTextPopup(HWND hWindow, PopupData data) {
	char* text = static_cast<char*>(data.userdata);

	HFONT arial = CreateFont(LARGE_FONT_HEIGHT, 0, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_SWISS, "Arial");

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

	if(data.textFlags & DT_SINGLELINE)
		DrawText(dc, text, strlen(text), &textArea, DT_CENTER | DT_VCENTER | data.textFlags);
	else { // manually center multiline text
		int textHeight = DrawText(dc, text, strlen(text), &textArea, DT_CALCRECT);
		int drawFromY = (POPUP_HEIGHT - WHITE_BAR_HEIGHT - textHeight) / 2;
		textArea = {
			0, drawFromY,
			POPUP_WIDTH, POPUP_HEIGHT - WHITE_BAR_HEIGHT - drawFromY
		};
		DrawText(dc, text, strlen(text), &textArea, DT_CENTER | data.textFlags);
	}

	DeleteObject(arial);
	EndPaint(hWindow, &ps2);
}


void displayCorrectPopup(HWND hWindow, PopupData data) {
	setWindowPosAndSize(hWindow, -1, -1, POPUP_WIDTH, POPUP_HEIGHT);

	switch (data.type) {
	case POPUP_SONGS:
		drawMultipleSongsPopup(hWindow, data);
		break;
	case POPUP_TEXT:
		drawTextPopup(hWindow, data);
		break;
	}
}