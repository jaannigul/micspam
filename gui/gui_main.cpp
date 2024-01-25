#include "gui_main.h"
#include "windowutil.h"
#include "popups.h"

#include <iostream>
#include <Windows.h>
#include <pthread.h>
#include <chrono>

StsHeader* popupTypesQueue = nullptr;
int popupX = 0;
int popupY = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    std::cout << msg << std::endl;

    switch (msg)
    {
    case WM_NCHITTEST:
        return HTCAPTION;
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
    HWND hWindow = createWindow(WndProc);
    if (!hWindow) return 0;

    std::chrono::steady_clock::time_point popupStartTime = { };
    bool isPopupVisible = false;

    MSG msg;
    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, TRUE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // handle popup animations
        isPopupVisible = handlePopupAnimation(hWindow, popupStartTime, isPopupVisible);

        PopupData* data = static_cast<PopupData*>(StsQueue.pop(popupTypesQueue));
        if (data == nullptr)
            continue;

        std::chrono::steady_clock::time_point popupStartTime = std::chrono::steady_clock::now();
        isPopupVisible = true;
        setWindowTransparency(hWindow, 255); // make the window visible again
        displayCorrectPopup(hWindow, *data, popupX, popupY);
    }

    return 0;
}

static const char* a = "MONKI!";

int guiTestEntryPoint() {
    popupTypesQueue = StsQueue.create();
    if (popupTypesQueue == NULL)
        return 1;

    PopupData* data = (PopupData*)malloc(sizeof(PopupData));
    data->type = POPUP_TEXT;
    data->userdata = const_cast<char*>(a);

    pthread_t thread;
    pthread_create(&thread, NULL, popupThread, NULL);
    pthread_detach(thread);

    Sleep(3000);

    StsQueue.push(popupTypesQueue, data, 0);

	return 0;
}