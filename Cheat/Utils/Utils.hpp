#pragma once

#define WIN32_LEAN_AND_MEAN
#define OPEN_MENU_KEY VK_RCONTROL

namespace Utils
{
    inline bool keyPressed(int vKey)
    {
        return (GetAsyncKeyState(vKey) & 1) != 0;
    }

    inline bool isGameWindowActive(HWND hwndGame)
    {
        HWND hwndForeground = GetForegroundWindow();

        if (hwndForeground != hwndGame)
            return false;

        if (IsIconic(hwndGame))
            return false;

        return true;
    }
}