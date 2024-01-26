#pragma once

#include <Windows.h>

EXTERN_C_START

void setWindowPosAndSize(HWND window, int xPos, int yPos, int width, int height);
void toggleWindowTransparency(HWND window, bool toggle);
void setWindowTransparency(HWND window, int alpha);
HWND createWindow(WNDPROC windowCallback);

EXTERN_C_END