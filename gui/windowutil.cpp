#include "windowutil.h"

#include <iostream>

const char* windowClass = "micspammerWindow";

// hacky solution that fixes the problem where our window will not change size properly
void setWindowPosAndSize(HWND window, int xPos, int yPos, int width, int height) {
    UINT flags = 0;
    if (xPos == -1 || yPos == -1)
        flags |= SWP_NOMOVE;

    SetWindowPos(window, HWND_TOP, xPos, yPos, width, height, flags);
}

void setWindowTransparency(HWND window, int alpha) {
    SetLayeredWindowAttributes(window, NULL, alpha, LWA_ALPHA);
    UpdateWindow(window);
}

void toggleWindowTransparency(HWND window, bool toggle) {
    LONG attrs = GetWindowLong(window, GWL_EXSTYLE);
    if (toggle)
        attrs |= WS_EX_LAYERED;
    else
        attrs &= ~WS_EX_LAYERED; // keep everything else except for this attr

    SetWindowLong(window, GWL_EXSTYLE, attrs);
}

HWND createWindow(WNDPROC windowCallback) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    //Step 1: Registering the Window Class
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = windowCallback;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(22, 22, 22));
    wc.lpszMenuName = NULL;
    wc.lpszClassName = windowClass;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    ATOM registerClassRes = RegisterClassEx(&wc);

    if (!registerClassRes)
    {
        std::cerr << "failed to register window class: " << GetLastError() << std::endl;
        return 0;
    }

    using fCreateWindowInBand = HWND(WINAPI*)(_In_ DWORD dwExStyle, _In_opt_ ATOM atom, _In_opt_ LPCWSTR lpWindowName, _In_ DWORD dwStyle, _In_ int X, _In_ int Y, _In_ int nWidth, _In_ int nHeight, _In_opt_ HWND hWndParent, _In_opt_ HMENU hMenu, _In_opt_ HINSTANCE hInstance, _In_opt_ LPVOID lpParam, DWORD band);

    HMODULE hLib = LoadLibrary("user32.dll");
    if (!hLib) {
        std::cerr << "failed to load user32.dll: " << GetLastError() << std::endl;
        return 0;
    }

    const auto pCreateWindowInBand = reinterpret_cast<fCreateWindowInBand>(GetProcAddress(hLib, "CreateWindowInBand"));

    hwnd = pCreateWindowInBand(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE, // NB! WS_EX_TRANSPARENT and WS_EX_NOACTIVATE are required so that we dont keep alt tabbing out of games
        registerClassRes,
        L"Title",
        WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        LPVOID(registerClassRes),
        2);

    FreeLibrary(hLib);

    if (hwnd == NULL)
    {
        std::cerr << "failed to create window" << std::endl;
        return 0;
    }

    return hwnd;
}