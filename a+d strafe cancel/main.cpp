#include <Windows.h>
#include <iostream>
#include <tray.hpp>

#include "icon_bytes.h"

bool KeyUp(int key)
{
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;
    input.ki.time = 123; // 123 for identification
    input.ki.dwExtraInfo = 0;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    return SendInput(1, &input, sizeof(input));
}

bool KeyDown(int key)
{
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;
    input.ki.time = 123; // 123 for identification
    input.ki.dwExtraInfo = 0;
    input.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
    return SendInput(1, &input, sizeof(input));
}

bool adown = false;
bool ddown = false;
bool close = false;
bool enabled = true;

HHOOK keyboard_hook;
LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && enabled && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN || wParam == WM_KEYUP || wParam == WM_SYSKEYUP) && ((KBDLLHOOKSTRUCT*)lParam)->time != 123)
    {
        bool down = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);

        switch (((KBDLLHOOKSTRUCT*)lParam)->vkCode)
        {
        case 'A':
            if (down)
            {
                adown = true;

                if (ddown && KeyUp('D'))
                    printf("D up\n");
            }
            else
            {
                adown = false;

                if (ddown)
                {
                    KeyDown('D');
                    printf("D down\n");
                }
            }
           
            break;

        case 'D':
            if (down)
            {
                ddown = true;

                if (adown && KeyUp('A'))
                    printf("A up\n");
            }
            else 
            {
                ddown = false;

                if (adown)
                {
                    KeyDown('A');
                    printf("A down\n");
                }
            }

            break;

        case VK_END:
            close = true;
            break;
        }
    }

    return CallNextHookEx(keyboard_hook, nCode, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#if 0
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    SetConsoleTitleA("A+D Null bind");
#endif

    keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, NULL, 0);
    if (keyboard_hook == NULL)
    {
        printf("Failed to create keyboard hook!\n");
        return 0;
    }

    HICON icon = CreateIconFromResourceEx(icon_bytes, sizeof(icon_bytes), TRUE, 0x30000, 22, 22, LR_DEFAULTCOLOR);
    if (!icon)
    {
        printf("Failed to create icon!\n");
        return 0;
    }

    Tray::Tray tray("Null Bind", icon);

    tray.addEntry(Tray::Toggle("Enable", enabled, [&](bool asd) { enabled = asd; }));
    tray.addEntry(Tray::Button("Exit (END Key)", [&] { close = true; }));

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT && !close)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
    }

    tray.exit();
    UnhookWindowsHookEx(keyboard_hook);

    return 0;
}