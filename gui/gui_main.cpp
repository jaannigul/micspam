#include "gui_main.h"
#include "windowutil.h"
#include "popups.h"

#include <iostream>
#include <Windows.h>
#include <pthread.h>
#include <chrono>

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
    HWND hWindow = createWindow(WndProc);
    if (!hWindow) return 0;

    std::chrono::steady_clock::time_point popupStartTime = { };

    MSG msg;
    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, TRUE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // handle popup animations
        handlePopupAnimation(hWindow, popupStartTime);

        PopupData* data = static_cast<PopupData*>(StsQueue.pop(popupTypesQueue));
        if (data == nullptr)
            continue;

        std::chrono::steady_clock::time_point popupStartTime = std::chrono::steady_clock::now();
        setWindowTransparency(hWindow, 255); // make the window visible again
        displayCorrectPopup(hWindow, *data);
    }

    return 0;
}

int guiTestEntryPoint() {
    popupTypesQueue = StsQueue.create();
    if (popupTypesQueue == NULL)
        return 1;

    PopupData* data = static_cast<PopupData*>(malloc(sizeof(PopupData)));
    data->type = POPUP_TEXT;
    data->userdata = (void*)"Volume set to 50%";

    pthread_t thread;
    pthread_create(&thread, NULL, popupThread, NULL);
    pthread_detach(thread);

    Sleep(3000);

    StsQueue.push(popupTypesQueue, data, 0);

	return 0;
}