#include "gui_main.h"
#include "windowutil.h"
#include "popups.h"

#include <iostream>
#include <Windows.h>
#include <pthread.h>
#include <chrono>
#include <queue>
#include <mutex>

std::queue<PopupData*> popupDataQueue;
std::mutex queueLock;

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
    PopupData* data = (PopupData*)malloc(sizeof(PopupData));
    if (!data) return;

    data->type = type;
    data->userdata = userdata;
    data->userdataCount = userdataCount;
    data->userdataIndex = userdataIndex;
    data->textFlags = textFlags;

    queueLock.lock();
    popupDataQueue.push(data);
    queueLock.unlock();
}

void* popupThread(void* arg) {
    HWND hWindow = createWindow(WndProc);
    if (!hWindow) return 0;
    ShowWindow(hWindow, SW_HIDE);

    std::chrono::steady_clock::time_point popupStartTime = { };
    bool isPopupVisible = false;

    MSG msg;
    PopupData savedData;
    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, TRUE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        isPopupVisible = handlePopupAnimation(hWindow, popupStartTime, isPopupVisible);

        queueLock.lock();
        if (popupDataQueue.empty()) {
            queueLock.unlock();
            continue;
        }
        PopupData* data = popupDataQueue.front();
        popupDataQueue.pop();
        queueLock.unlock();

        savedData = *data;
        free(data);

        popupStartTime = std::chrono::steady_clock::now();
        isPopupVisible = true;
        toggleWindowTransparency(hWindow, false);
        setWindowTransparency(hWindow, 255); // make the window visible again
        ShowWindow(hWindow, SW_SHOW);

        if (savedData.type != POPUP_KEEP_AWAKE) // only redraw the window when needed
            displayCorrectPopup(hWindow, savedData);
    }

    return 0;
}

int guiTestEntryPoint() {
    pthread_t thread;
    pthread_create(&thread, NULL, popupThread, NULL);
    pthread_detach(thread);

	return 0;
}