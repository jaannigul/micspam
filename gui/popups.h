#pragma once

#include <Windows.h>
#include <chrono>

void handlePopupAnimation(HWND hWindow, std::chrono::steady_clock::time_point popupStartTime);