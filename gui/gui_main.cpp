#include "gui_main.h"
#include "windowutil.h"
#include "popups.h"

#include <iostream>
#include <Windows.h>
#include <pthread.h>
#include <chrono>

static StsHeader* popupTypesQueue = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_NCHITTEST:
        return HTCAPTION;
    case WM_MOUSEMOVE: case WM_LBUTTONDOWN:
        toggleWindowTransparency(hwnd, false);
        return DefWindowProc(hwnd, msg, wParam, lParam);
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

void sendPopupNotification(enum PopupType type, void* userdata, int userdataCount, int userdataIndex, int textFlags) {
    if (popupTypesQueue == nullptr) return;
    StsHeader* copy = popupTypesQueue; // this somehow fixes crashing issue.... what the fuck

    PopupData* data = (PopupData*)malloc(sizeof(PopupData));
    if (!data) return;

    data->type = type;
    data->userdata = userdata;
    data->userdataCount = userdataCount;
    data->userdataIndex = userdataIndex;
    data->textFlags = textFlags;

    StsQueue.push(copy, data, 0, FALSE);
}

void* popupThread(void* arg) {
    HWND hWindow = createWindow(WndProc);
    if (!hWindow) return 0;

    std::chrono::steady_clock::time_point popupStartTime = { };
    bool isPopupVisible = false;

    MSG msg;
    PopupData savedData;
    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, TRUE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        StsHeader* popupTypesQueueLocal = static_cast<StsHeader*>(arg); // this magically fixes a crash that occurs when we stop playing aujdio, and the popupTypesQueue wants to pop something

        // handle popup animations
        isPopupVisible = handlePopupAnimation(hWindow, popupStartTime, isPopupVisible);

        PopupData* data = static_cast<PopupData*>(StsQueue.pop(popupTypesQueueLocal));
        if (data == nullptr)
            continue;

        savedData = *data;
        free(data);

        popupStartTime = std::chrono::steady_clock::now();
        isPopupVisible = true;
        toggleWindowTransparency(hWindow, false);
        setWindowTransparency(hWindow, 255); // make the window visible again
        ShowWindow(hWindow, SW_SHOW);

        if(savedData.type != POPUP_KEEP_AWAKE) // only redraw the window when needed
            displayCorrectPopup(hWindow, savedData);
    }

    return 0;
}

int guiTestEntryPoint() {
    popupTypesQueue = StsQueue.create();
    if (popupTypesQueue == NULL)
        return 1;

    pthread_t thread;
    pthread_create(&thread, NULL, popupThread, popupTypesQueue);
    pthread_detach(thread);

	return 0;
}