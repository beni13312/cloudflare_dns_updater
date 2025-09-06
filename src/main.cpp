#include "cflare.h"

// WINDOWS
#if defined(_WIN32)
#include <windows.h>
#include <shellapi.h>
#include <iomanip>
#include <chrono>
#include <thread>
#include <atomic>

// Global variables
NOTIFYICONDATA nid;
HMENU hMenu;
UINT_PTR timerID; // Timer ID for scheduling
std::atomic runCflareTask(true); // Flag to control the background task


// Window procedure function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_USER + 1: // Custom message for system tray interactions
            if (LOWORD(lParam) == WM_RBUTTONUP) { // Right-click on the icon
                POINT pt;
                GetCursorPos(&pt);
                SetForegroundWindow(hwnd);
                // Show the context menu at the cursor position
                TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
            }
            break;

        case WM_COMMAND: // Handle commands from the context menu
            switch (LOWORD(wParam)) {
                case 1: // "Open" menu item
                    ShellExecute(NULL, "open", "notepad", "cflare.json", NULL, SW_SHOWNORMAL);
                    break;
                case 2: // "Exit" menu item
                    Shell_NotifyIcon(NIM_DELETE, &nid);
                    runCflareTask = false; // Stop the background task
                    PostQuitMessage(0);
                    break;
            }
            break;

        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void cflareTask() {
    // Run the cflare function in a loop
    cflare::run();
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("TrayIconClass");

    // Register the window class
    RegisterClass(&wc);

    // Create a hidden window
    HWND hwnd = CreateWindowEx(
        0, TEXT("TrayIconClass"), TEXT(""), 0,
        0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

    if (!hwnd) {
        return 1; // Failed to create hidden window
    }

    // Set up NOTIFYICONDATA structure
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon = (HICON)LoadImage(NULL, TEXT("icon.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE); // application icon
    lstrcpyn(nid.szTip, TEXT("cflare"), sizeof(nid.szTip) / sizeof(nid.szTip[0]));

    // Add the icon to the system tray
    Shell_NotifyIcon(NIM_ADD, &nid);

    // Create a context menu
    hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, 1, TEXT("Change settings"));
    AppendMenu(hMenu, MF_STRING, 2, TEXT("Exit"));

    // Start the cflare task in a separate thread
    std::thread cflareThread(cflareTask);
    cflareThread.detach();

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Ensure the task thread is stopped if the user exits the application
    runCflareTask = false;

    return 0;
}


// LINUX
#elif defined(__linux__)
int main() {
    cflare::run();
    return 0;
}
#else
int main() {
    return 0;
}
#endif
