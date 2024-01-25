#include "gui_main.h"
#include "windowutil.h"

#include <iostream>
#include <Windows.h>
#include <pthread.h>

StsHeader* popupTypesQueue = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void* __cdecl popupThread(void* arg) {
    HWND hWindow = static_cast<HWND>(arg);

    while (true) {
        Sleep(1);

        int* popupType = static_cast<int*>(StsQueue.pop(popupTypesQueue));
        if (popupType == nullptr)
            continue;

    }

    return 0;
}

int guiTestEntryPoint() {
    HWND hWindow = createWindow(WndProc);
    if (!hWindow) return 1;

    pthread_t thread;
    pthread_create(&thread, NULL, popupThread, hWindow);
    pthread_detach(thread);

	return 0;
}