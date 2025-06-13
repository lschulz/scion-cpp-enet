// Copyright (c) 2024-2025 Lars-Christian Schulz
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef min
#undef max

#define CON_CURSES(x)
#define CON_WIN32(x) x

bool isKeyPressed(HANDLE hConsoleInput, WORD vKey)
{
    DWORD eventCount = 0;
    INPUT_RECORD buffer[8];
    while (true) {
        GetNumberOfConsoleInputEvents(hConsoleInput, &eventCount);
        if (eventCount == 0) break;
        DWORD recordsRead = 0;
        ReadConsoleInput(hConsoleInput, buffer, ARRAYSIZE(buffer), &recordsRead);
        for (DWORD i = 0; i < recordsRead; ++i) {
            if (buffer[i].EventType == KEY_EVENT) {
                const auto &keyEvent = buffer[i].Event.KeyEvent;
                if (keyEvent.bKeyDown && keyEvent.wVirtualKeyCode == vKey)
                    return true;
            }
        }
    }
    return false;
}

#else

// ncurses wrappers (to prevent ncurses from leaking into the global namespace)
namespace ncurses {
void initServer();
void refreshScreen();
void print(const char* str);
int getChar();
void endServer();
}

#define CON_CURSES(x) x
#define CON_WIN32(x)

#endif
