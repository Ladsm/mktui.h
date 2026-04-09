/*
 *  ██████   ██████  █████   ████  ███████████  █████  █████  █████      █████   █████
 * ░░██████ ██████  ░░███   ███░  ░█░░░███░░░█ ░░███  ░░███  ░░███      ░░███   ░░███
 *  ░███░█████░███   ░███  ███    ░   ░███  ░   ░███   ░███   ░███       ░███    ░███
 *  ░███░░███ ░███   ░███████         ░███      ░███   ░███   ░███       ░███████████
 *  ░███ ░░░  ░███   ░███░░███        ░███      ░███   ░███   ░███       ░███░░░░░███
 *  ░███      ░███   ░███ ░░███       ░███      ░███   ░███   ░███       ░███    ░███
 *  █████     █████  █████ ░░████     █████     ░░████████    █████  ██  █████   █████
 * ░░░░░     ░░░░░  ░░░░░   ░░░░     ░░░░░       ░░░░░░░░    ░░░░░  ░░  ░░░░░   ░░░░░
 *
 *  mktui — Minimal Cross-Platform TUI Utilities
 *  ------------------------------------------------------------
 *  A lightweight, single-header C++ utility for building
 *  terminal user interfaces (TUIs).
 *
 *  It fetures:
 *   - Cross-platform (Windows / Linux / Unix)
 *   - Keyboard + mouse input handling
 *   - Console control (cursor, screen, title, etc.)
 *   - ANSI colors (8 / 16 / 24-bit)
 *   - No dependencies
 *
 *  Philosophy
 *  ----------
 *  mktui does the low-level, annoying console work so you don’t have to.
 *  It avoids opinionated abstractions and stays out of your way.
 *
 *  - No frameworks
 *  - No forced rendering model
 *  - No std::cout abuse (only ANSI/control sequences when needed)
 *
 *  You build the UI. mktui gives you the tools.
 *
 *  Usage Notes
 *  -----------
 *  - Use mktui::console_guard for proper setup/cleanup
 *  - Do not mix with other terminal control libraries
 *  - Designed for real-time / interactive console apps
 *
 *  License
 *  -------
 *
 * MIT License
 *
 * Copyright (c) 2026 Ladsm(https://github.com/ladsm/)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#if defined(_WIN32)
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <poll.h>
#endif
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>
#include <cstring>
#include <cstdio>
#include <memory>

namespace mktui {
#ifndef _WIN32
    inline static struct termios originalTermios;
#endif
    inline int get_Console_Width() {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            return csbi.srWindow.Right - csbi.srWindow.Left + 1;
        }
#else
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) { return w.ws_col; }
#endif
        return 80;
    }
    inline int get_Console_Height() {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        }
#else
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) { return w.ws_row; }
#endif
        return 25;
    }
    enum class Input_Type {
        None,
        MoveUp,
        MoveDown,
        MoveLeft,
        MoveRight,
        Top0,
        Top1,
        Top2,
        Top3,
        Top4,
        Top5,
        Top6,
        Top7,
        Top8,
        Top9,
        Enter,
        Space,
        Escape,
        Tab,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        Q, W, E, R, T, Y, U, I, O, P,
        A, S, D, F, G, H, J, K, L,
        Z, X, C, V, B, N, M,
        MouseMove,
        MouseLeftDown,
        MouseLeftUp,
        MouseRightDown,
        MouseRightUp
    };
#ifndef _WIN32
    int getch() {
        return getchar();
    }
#endif
    inline int g_mouseX = 0;
    inline int g_mouseY = 0;
    inline int get_MouseX() { return g_mouseX; }
    inline int get_MouseY() { return g_mouseY; }
    inline Input_Type Get_Input() {
#if defined(_WIN32)
        static bool consoleInitialized = false;
        static DWORD originalMode = 0;
        HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
        if (!consoleInitialized) {
            if (GetConsoleMode(hIn, &originalMode)) {
                DWORD mode = originalMode;
                mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
                mode |= ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
                SetConsoleMode(hIn, mode);
            }
            consoleInitialized = true;
        }
        INPUT_RECORD ir;
        DWORD read = 0;
        static DWORD prevMouseButtons = 0;
        while (true) {
            if (!ReadConsoleInput(hIn, &ir, 1, &read)) {
                int ch = _getch();
                if (ch == 9) return Input_Type::Tab;
                if (ch == 13) return Input_Type::Enter;
                if (ch == ' ') return Input_Type::Space;
                if (ch == 27) return Input_Type::Escape;
                if (ch == 0 || ch == 224) {
                    int ex = _getch();
                    if (ex >= 59 && ex <= 68) return static_cast<Input_Type>((int)Input_Type::F1 + (ex - 59));
                    if (ex == 133) return Input_Type::F11;
                    if (ex == 134) return Input_Type::F12;
                }
                if (ch >= '0' && ch <= '9') return static_cast<Input_Type>(static_cast<int>(Input_Type::Top0) + (ch - '0'));
                switch (ch) {
                case 'q': case 'Q': return Input_Type::Q;
                case 'w': case 'W': return Input_Type::W;
                case 'e': case 'E': return Input_Type::E;
                case 'r': case 'R': return Input_Type::R;
                case 't': case 'T': return Input_Type::T;
                case 'y': case 'Y': return Input_Type::Y;
                case 'u': case 'U': return Input_Type::U;
                case 'i': case 'I': return Input_Type::I;
                case 'o': case 'O': return Input_Type::O;
                case 'p': case 'P': return Input_Type::P;
                case 'a': case 'A': return Input_Type::A;
                case 's': case 'S': return Input_Type::S;
                case 'd': case 'D': return Input_Type::D;
                case 'f': case 'F': return Input_Type::F;
                case 'g': case 'G': return Input_Type::G;
                case 'h': case 'H': return Input_Type::H;
                case 'j': case 'J': return Input_Type::J;
                case 'k': case 'K': return Input_Type::K;
                case 'l': case 'L': return Input_Type::L;
                case 'z': case 'Z': return Input_Type::Z;
                case 'x': case 'X': return Input_Type::X;
                case 'c': case 'C': return Input_Type::C;
                case 'v': case 'V': return Input_Type::V;
                case 'b': case 'B': return Input_Type::B;
                case 'n': case 'N': return Input_Type::N;
                case 'm': case 'M': return Input_Type::M;
                default: return Input_Type::None;
                }
            }
            if (ir.EventType == KEY_EVENT) {
                auto& ke = ir.Event.KeyEvent;
                if (!ke.bKeyDown) continue;
                if (ke.wVirtualKeyCode == VK_TAB) return Input_Type::Tab;
                if (ke.wVirtualKeyCode >= VK_F1 && ke.wVirtualKeyCode <= VK_F12) {
                    return static_cast<Input_Type>((int)Input_Type::F1 + (ke.wVirtualKeyCode - VK_F1));
                }
                char ch = (char)ke.uChar.AsciiChar;
                if (ch == 13) return Input_Type::Enter;
                if (ch == ' ') return Input_Type::Space;
                if (ch == 27) return Input_Type::Escape;
                switch (ke.wVirtualKeyCode) {
                case VK_UP:    return Input_Type::MoveUp;
                case VK_DOWN:  return Input_Type::MoveDown;
                case VK_LEFT:  return Input_Type::MoveLeft;
                case VK_RIGHT: return Input_Type::MoveRight;
                }
                if (ch >= '0' && ch <= '9') return static_cast<Input_Type>(static_cast<int>(Input_Type::Top0) + (ch - '0'));
                switch (ch) {
                case 'q': case 'Q': return Input_Type::Q;
                case 'w': case 'W': return Input_Type::W;
                case 'e': case 'E': return Input_Type::E;
                case 'r': case 'R': return Input_Type::R;
                case 't': case 'T': return Input_Type::T;
                case 'y': case 'Y': return Input_Type::Y;
                case 'u': case 'U': return Input_Type::U;
                case 'i': case 'I': return Input_Type::I;
                case 'o': case 'O': return Input_Type::O;
                case 'p': case 'P': return Input_Type::P;
                case 'a': case 'A': return Input_Type::A;
                case 's': case 'S': return Input_Type::S;
                case 'd': case 'D': return Input_Type::D;
                case 'f': case 'F': return Input_Type::F;
                case 'g': case 'G': return Input_Type::G;
                case 'h': case 'H': return Input_Type::H;
                case 'j': case 'J': return Input_Type::J;
                case 'k': case 'K': return Input_Type::K;
                case 'l': case 'L': return Input_Type::L;
                case 'z': case 'Z': return Input_Type::Z;
                case 'x': case 'X': return Input_Type::X;
                case 'c': case 'C': return Input_Type::C;
                case 'v': case 'V': return Input_Type::V;
                case 'b': case 'B': return Input_Type::B;
                case 'n': case 'N': return Input_Type::N;
                case 'm': case 'M': return Input_Type::M;
                default: return Input_Type::None;
                }
            }
            else if (ir.EventType == MOUSE_EVENT) {
                auto& me = ir.Event.MouseEvent;
                g_mouseX = me.dwMousePosition.X;
                g_mouseY = me.dwMousePosition.Y;
                DWORD btns = me.dwButtonState;
                if (me.dwEventFlags == MOUSE_MOVED) {
                    prevMouseButtons = btns;
                    return Input_Type::MouseMove;
                }
                DWORD leftMask = FROM_LEFT_1ST_BUTTON_PRESSED;
                bool prevLeft = (prevMouseButtons & leftMask) != 0;
                bool curLeft = (btns & leftMask) != 0;
                prevMouseButtons = btns;
                if (!prevLeft && curLeft) return Input_Type::MouseLeftDown;
                if (prevLeft && !curLeft) return Input_Type::MouseLeftUp;
                return Input_Type::MouseMove;
            }
        }
#else
        static bool consoleInitialized = false;
        if (!consoleInitialized) {
            tcgetattr(STDIN_FILENO, &originalTermios);
            struct termios raw = originalTermios;
            raw.c_lflag &= ~(ICANON | ECHO);
            raw.c_iflag &= ~(IXON | ICRNL);
            tcsetattr(STDIN_FILENO, TCSANOW, &raw);
            std::cout << "\033[?1002h\033[?1006h" << std::flush;
            consoleInitialized = true;
        }
        int ch = getch();
        if (ch == 9) return Input_Type::Tab;
        if (ch == 27) {
            int n1 = getch();
            if (n1 == '[') {
                int n2 = getch();
                if (n2 == '<') {
                    int cb = 0, cx = 0, cy = 0;
                    int c;
                    while ((c = getch()) >= '0' && c <= '9') { cb = cb * 10 + (c - '0'); }
                    if (c == ';') {
                        while ((c = getch()) >= '0' && c <= '9') { cx = cx * 10 + (c - '0'); }
                        if (c == ';') {
                            while ((c = getch()) >= '0' && c <= '9') { cy = cy * 10 + (c - '0'); }
                            if (c == 'M' || c == 'm') {
                                g_mouseX = cx > 0 ? cx - 1 : 0;
                                g_mouseY = cy > 0 ? cy - 1 : 0;
                                bool isMotion = (cb & 32) != 0;
                                int button = cb & 0b11;
                                if (isMotion) return Input_Type::MouseMove;
                                if (c == 'M') {
                                    if (button == 0) return Input_Type::MouseLeftDown;
                                    return Input_Type::MouseMove;
                                }
                                else {
                                    if (button == 0 || button == 3) return Input_Type::MouseLeftUp;
                                    return Input_Type::MouseMove;
                                }
                            }
                        }
                    }
                }
                else {
                    if (n2 == 'A') return Input_Type::MoveUp;
                    if (n2 == 'B') return Input_Type::MoveDown;
                    if (n2 == 'C') return Input_Type::MoveRight;
                    if (n2 == 'D') return Input_Type::MoveLeft;
                    if (n2 >= '0' && n2 <= '9') {
                        int num = n2 - '0';
                        int next;
                        while ((next = getch()) >= '0' && next <= '9') num = num * 10 + (next - '0');
                        if (num == 15) return Input_Type::F5;
                        if (num == 17) return Input_Type::F6;
                        if (num == 18) return Input_Type::F7;
                        if (num == 19) return Input_Type::F8;
                        if (num == 20) return Input_Type::F9;
                        if (num == 21) return Input_Type::F10;
                        if (num == 23) return Input_Type::F11;
                        if (num == 24) return Input_Type::F12;
                    }
                }
            }
            else if (n1 == 'O') {
                int n2 = getch();
                if (n2 == 'P') return Input_Type::F1;
                if (n2 == 'Q') return Input_Type::F2;
                if (n2 == 'R') return Input_Type::F3;
                if (n2 == 'S') return Input_Type::F4;
            }
            return Input_Type::Escape;
        }
        if (ch == 10 || ch == 13) return Input_Type::Enter;
        if (ch == ' ') return Input_Type::Space;
        if (ch >= '0' && ch <= '9') return static_cast<Input_Type>(static_cast<int>(Input_Type::Top0) + (ch - '0'));
        switch (ch) {
        case 'q': case 'Q': return Input_Type::Q;
        case 'w': case 'W': return Input_Type::W;
        case 'e': case 'E': return Input_Type::E;
        case 'r': case 'R': return Input_Type::R;
        case 't': case 'T': return Input_Type::T;
        case 'y': case 'Y': return Input_Type::Y;
        case 'u': case 'U': return Input_Type::U;
        case 'i': case 'I': return Input_Type::I;
        case 'o': case 'O': return Input_Type::O;
        case 'p': case 'P': return Input_Type::P;
        case 'a': case 'A': return Input_Type::A;
        case 's': case 'S': return Input_Type::S;
        case 'd': case 'D': return Input_Type::D;
        case 'f': case 'F': return Input_Type::F;
        case 'g': case 'G': return Input_Type::G;
        case 'h': case 'H': return Input_Type::H;
        case 'j': case 'J': return Input_Type::J;
        case 'k': case 'K': return Input_Type::K;
        case 'l': case 'L': return Input_Type::L;
        case 'z': case 'Z': return Input_Type::Z;
        case 'x': case 'X': return Input_Type::X;
        case 'c': case 'C': return Input_Type::C;
        case 'v': case 'V': return Input_Type::V;
        case 'b': case 'B': return Input_Type::B;
        case 'n': case 'N': return Input_Type::N;
        case 'm': case 'M': return Input_Type::M;
        }
        return Input_Type::None;
#endif
    }
    inline Input_Type Get_Input_Nonblocking() {
#if defined(_WIN32)
        static bool consoleInitialized = false;
        static DWORD originalMode = 0;
        HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
        if (!consoleInitialized) {
            if (GetConsoleMode(hIn, &originalMode)) {
                DWORD mode = originalMode;
                mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
                mode |= ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
                SetConsoleMode(hIn, mode);
            }
            consoleInitialized = true;
        }
        DWORD eventsWaiting = 0;
        if (!GetNumberOfConsoleInputEvents(hIn, &eventsWaiting) || eventsWaiting == 0) {
            return Input_Type::None;
        }
        INPUT_RECORD ir;
        DWORD read = 0;
        static DWORD prevMouseButtons = 0;
        if (!ReadConsoleInput(hIn, &ir, 1, &read)) return Input_Type::None;

        if (ir.EventType == KEY_EVENT) {
            auto& ke = ir.Event.KeyEvent;
            if (!ke.bKeyDown) return Input_Type::None;

            if (ke.wVirtualKeyCode == VK_TAB) return Input_Type::Tab;
            if (ke.wVirtualKeyCode >= VK_F1 && ke.wVirtualKeyCode <= VK_F12) {
                return static_cast<Input_Type>((int)Input_Type::F1 + (ke.wVirtualKeyCode - VK_F1));
            }
            char ch = (char)ke.uChar.AsciiChar;
            if (ch == 13) return Input_Type::Enter;
            if (ch == ' ') return Input_Type::Space;
            if (ch == 27) return Input_Type::Escape;
            switch (ke.wVirtualKeyCode) {
            case VK_UP:    return Input_Type::MoveUp;
            case VK_DOWN:  return Input_Type::MoveDown;
            case VK_LEFT:  return Input_Type::MoveLeft;
            case VK_RIGHT: return Input_Type::MoveRight;
            }
            if (ch >= '0' && ch <= '9') return static_cast<Input_Type>(static_cast<int>(Input_Type::Top0) + (ch - '0'));
            switch (tolower(ch)) {
            case 'q': case 'Q': return Input_Type::Q;
            case 'w': case 'W': return Input_Type::W;
            case 'e': case 'E': return Input_Type::E;
            case 'r': case 'R': return Input_Type::R;
            case 't': case 'T': return Input_Type::T;
            case 'y': case 'Y': return Input_Type::Y;
            case 'u': case 'U': return Input_Type::U;
            case 'i': case 'I': return Input_Type::I;
            case 'o': case 'O': return Input_Type::O;
            case 'p': case 'P': return Input_Type::P;
            case 'a': case 'A': return Input_Type::A;
            case 's': case 'S': return Input_Type::S;
            case 'd': case 'D': return Input_Type::D;
            case 'f': case 'F': return Input_Type::F;
            case 'g': case 'G': return Input_Type::G;
            case 'h': case 'H': return Input_Type::H;
            case 'j': case 'J': return Input_Type::J;
            case 'k': case 'K': return Input_Type::K;
            case 'l': case 'L': return Input_Type::L;
            case 'z': case 'Z': return Input_Type::Z;
            case 'x': case 'X': return Input_Type::X;
            case 'c': case 'C': return Input_Type::C;
            case 'v': case 'V': return Input_Type::V;
            case 'b': case 'B': return Input_Type::B;
            case 'n': case 'N': return Input_Type::N;
            case 'm': case 'M': return Input_Type::M;
            }
        }
        else if (ir.EventType == MOUSE_EVENT) {
            auto& me = ir.Event.MouseEvent;
            g_mouseX = me.dwMousePosition.X;
            g_mouseY = me.dwMousePosition.Y;
            DWORD btns = me.dwButtonState;
            if (me.dwEventFlags == MOUSE_MOVED) {
                prevMouseButtons = btns;
                return Input_Type::MouseMove;
            }
            DWORD leftMask = FROM_LEFT_1ST_BUTTON_PRESSED;
            bool prevLeft = (prevMouseButtons & leftMask) != 0;
            bool curLeft = (btns & leftMask) != 0;
            prevMouseButtons = btns;
            if (!prevLeft && curLeft) return Input_Type::MouseLeftDown;
            if (prevLeft && !curLeft) return Input_Type::MouseLeftUp;

            return Input_Type::MouseMove;
        }
        return Input_Type::None;

#else
        static bool consoleInitialized = false;
        if (!consoleInitialized) {
            tcgetattr(STDIN_FILENO, &originalTermios);
            struct termios raw = originalTermios;
            raw.c_lflag &= ~(ICANON | ECHO);
            raw.c_iflag &= ~(IXON | ICRNL);
            tcsetattr(STDIN_FILENO, TCSANOW, &raw);
            std::cout << "\033[?1002h\033[?1006h" << std::flush;
            consoleInitialized = true;
        }
        struct pollfd pfd = { STDIN_FILENO, POLLIN, 0 };
        if (poll(&pfd, 1, 0) <= 0) {
            return Input_Type::None;
        }
        int ch = getch();
        if (ch == 9) return Input_Type::Tab;
        if (ch == 27) {
            struct pollfd seq_pfd = { STDIN_FILENO, POLLIN, 0 };
            if (poll(&seq_pfd, 1, 50) <= 0) return Input_Type::Escape;
            int n1 = getch();
            if (n1 == '[') {
                int n2 = getch();
                if (n2 == '<') {
                    int cb = 0, cx = 0, cy = 0;
                    int c;
                    while ((c = getch()) >= '0' && c <= '9') { cb = cb * 10 + (c - '0'); }
                    if (c == ';') {
                        while ((c = getch()) >= '0' && c <= '9') { cx = cx * 10 + (c - '0'); }
                        if (c == ';') {
                            while ((c = getch()) >= '0' && c <= '9') { cy = cy * 10 + (c - '0'); }
                            if (c == 'M' || c == 'm') {
                                g_mouseX = cx > 0 ? cx - 1 : 0;
                                g_mouseY = cy > 0 ? cy - 1 : 0;
                                if ((cb & 32) != 0) return Input_Type::MouseMove;
                                int button = cb & 0b11;
                                if (c == 'M') return (button == 0) ? Input_Type::MouseLeftDown : Input_Type::MouseMove;
                                return (button == 0 || button == 3) ? Input_Type::MouseLeftUp : Input_Type::MouseMove;
                            }
                        }
                    }
                }
                else {
                    if (n2 == 'A') return Input_Type::MoveUp;
                    if (n2 == 'B') return Input_Type::MoveDown;
                    if (n2 == 'C') return Input_Type::MoveRight;
                    if (n2 == 'D') return Input_Type::MoveLeft;
                    if (n2 >= '0' && n2 <= '9') {
                        int num = n2 - '0';
                        int next;
                        while ((next = getch()) >= '0' && next <= '9') num = num * 10 + (next - '0');
                        switch (num) {
                        case 15: return Input_Type::F5; case 17: return Input_Type::F6;
                        case 18: return Input_Type::F7; case 19: return Input_Type::F8;
                        case 20: return Input_Type::F9; case 21: return Input_Type::F10;
                        case 23: return Input_Type::F11; case 24: return Input_Type::F12;
                        }
                    }
                }
            }
            else if (n1 == 'O') {
                int n2 = getch();
                if (n2 == 'P') return Input_Type::F1;
                if (n2 == 'Q') return Input_Type::F2;
                if (n2 == 'R') return Input_Type::F3;
                if (n2 == 'S') return Input_Type::F4;
            }
            return Input_Type::Escape;
        }
        if (ch == 10 || ch == 13) return Input_Type::Enter;
        if (ch == ' ') return Input_Type::Space;
        if (ch >= '0' && ch <= '9') return static_cast<Input_Type>(static_cast<int>(Input_Type::Top0) + (ch - '0'));
        switch (tolower(ch)) {
        case 'q': case 'Q': return Input_Type::Q;
        case 'w': case 'W': return Input_Type::W;
        case 'e': case 'E': return Input_Type::E;
        case 'r': case 'R': return Input_Type::R;
        case 't': case 'T': return Input_Type::T;
        case 'y': case 'Y': return Input_Type::Y;
        case 'u': case 'U': return Input_Type::U;
        case 'i': case 'I': return Input_Type::I;
        case 'o': case 'O': return Input_Type::O;
        case 'p': case 'P': return Input_Type::P;
        case 'a': case 'A': return Input_Type::A;
        case 's': case 'S': return Input_Type::S;
        case 'd': case 'D': return Input_Type::D;
        case 'f': case 'F': return Input_Type::F;
        case 'g': case 'G': return Input_Type::G;
        case 'h': case 'H': return Input_Type::H;
        case 'j': case 'J': return Input_Type::J;
        case 'k': case 'K': return Input_Type::K;
        case 'l': case 'L': return Input_Type::L;
        case 'z': case 'Z': return Input_Type::Z;
        case 'x': case 'X': return Input_Type::X;
        case 'c': case 'C': return Input_Type::C;
        case 'v': case 'V': return Input_Type::V;
        case 'b': case 'B': return Input_Type::B;
        case 'n': case 'N': return Input_Type::N;
        case 'm': case 'M': return Input_Type::M;
        }
        return Input_Type::None;
#endif
    }
#if defined(_WIN32)
    inline int read_Key() {
        int ch = _getch();
        if (ch == 0 || ch == 224) {
            int ch2 = _getch();
            return 1000 + ch2;
        }
        return ch;
    }
#else
    inline int read_Key() {
        termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        int ch = getchar();
        if (ch == 27) {
            if (getchar() == '[') {
                ch = 1000 + getchar();
            }
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }
#endif
    /*
     * Warning:
     *   - Console state is modified when using mktui::console_guard.
     *   - Avoid mixing with other terminal manipulation libraries.
     *   - Always keep console_guard in scope while running your TUI.
     */
    struct console_guard {
        console_guard() {
#if defined(_WIN32)
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hOut != INVALID_HANDLE_VALUE) {
                DWORD mode = 0;
                if (GetConsoleMode(hOut, &mode)) {
                    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                    SetConsoleMode(hOut, mode);
                }
            }
            SetConsoleOutputCP(CP_UTF8);
            SetConsoleCP(CP_UTF8);
#else
            tcgetattr(STDIN_FILENO, &originalTermios);
            struct termios raw = originalTermios;
            raw.c_lflag &= ~(ICANON | ECHO);
            raw.c_iflag &= ~(IXON | ICRNL);
            tcsetattr(STDIN_FILENO, TCSANOW, &raw);
#endif
            std::cout << "\033[?25l\033[?1000h\033[?1002h\033[?1006h\033[2J\033[H\033[?1049h\033[?25l";
            std::cout.flush();
            std::atexit(reset_console);
        }
        ~console_guard() {
            reset_console();
        }
    private:
        static void reset_console() {
            static bool cleaned = false;
            if (cleaned) return;
#ifndef _WIN32
            tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios);
#endif
            std::cout << "\033[?25h\033[?1000l\033[?1002l\033[?1006l\033[?1049l\033[2J\033[H\033[!p" << std::flush;
            cleaned = true;
        }
    };
    namespace colors {
        namespace bit8 {
            inline const std::string reset = "\033[0m";
            inline const std::string black = "\033[30m";
            inline const std::string red = "\033[31m";
            inline const std::string green = "\033[32m";
            inline const std::string yellow = "\033[33m";
            inline const std::string blue = "\033[34m";
            inline const std::string magenta = "\033[35m";
            inline const std::string cyan = "\033[36m";
            inline const std::string white = "\033[37m";
            inline const std::string bg_black = "\033[40m";
            inline const std::string bg_red = "\033[41m";
            inline const std::string bg_green = "\033[42m";
            inline const std::string bg_yellow = "\033[43m";
            inline const std::string bg_blue = "\033[44m";
            inline const std::string bg_magenta = "\033[45m";
            inline const std::string bg_cyan = "\033[46m";
            inline const std::string bg_white = "\033[47m";
        }
        namespace bit16 {
            inline const std::string black = "\033[90m";
            inline const std::string red = "\033[91m";
            inline const std::string green = "\033[92m";
            inline const std::string yellow = "\033[93m";
            inline const std::string blue = "\033[94m";
            inline const std::string magenta = "\033[95m";
            inline const std::string cyan = "\033[96m";
            inline const std::string white = "\033[97m";
            inline const std::string bg_black = "\033[100m";
            inline const std::string bg_red = "\033[101m";
            inline const std::string bg_green = "\033[102m";
            inline const std::string bg_yellow = "\033[103m";
            inline const std::string bg_blue = "\033[104m";
            inline const std::string bg_magenta = "\033[105m";
            inline const std::string bg_cyan = "\033[106m";
            inline const std::string bg_white = "\033[107m";
        }
        namespace bit24 {
            inline std::string fg(int r, int g, int b) {
                return "\033[38;2;" + std::to_string(r) + ";" +
                    std::to_string(g) + ";" + std::to_string(b) + "m";
            }
            inline std::string bg(int r, int g, int b) {
                return "\033[48;2;" + std::to_string(r) + ";" +
                    std::to_string(g) + ";" + std::to_string(b) + "m";
            }
        }
    }
    namespace attrs {
        inline const std::string reset = "\033[0m";
        inline const std::string bold = "\033[1m";
        inline const std::string italic = "\033[3m";
        inline const std::string underline = "\033[4m";
        inline const std::string inverse = "\033[7m";
        inline const std::string hidden = "\033[8m";
        inline const std::string blink = "\033[5m";
    }
    inline void set_cursor(int x, int y) {
        std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H";
    }
    inline void clear_screen() {
        std::cout << "\033[2J\033[H";
    }
    inline void clear_line() {
        std::cout << "\033[2K\r";
    }
    inline void set_title(const std::string& title) {
        std::cout << "\033]0;" << title << "\007";
    }
    inline void sleep_ms(int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    inline void sleep_sec(int s) {
        std::this_thread::sleep_for(std::chrono::seconds(s));
    }
    inline void cursor_move_up(int n = 1) { std::cout << "\033[" << n << "A"; }
    inline void cursor_move_down(int n = 1) { std::cout << "\033[" << n << "B"; }
    inline void cursor_move_right(int n = 1) { std::cout << "\033[" << n << "C"; }
    inline void cursor_move_left(int n = 1) { std::cout << "\033[" << n << "D"; }
    inline void debug_log(const std::string& msg) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::cerr << "[" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << "] " << "[DEBUG] " << msg << "\n" << std::flush;
    }
#ifndef _WIN32
    inline bool command_exists(const std::string& cmd) {
        std::string check = "command -v " + cmd + " > /dev/null 2>&1";
        return (system(check.c_str()) == 0);
    }
#endif
    inline void copy_to_clipboard(const std::string& text) {
#ifdef _WIN32
        if (!OpenClipboard(nullptr)) return;
        EmptyClipboard();
        size_t size = text.size() + 1;
        HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, size);
        if (hGlob) {
            void* lock = GlobalLock(hGlob);
            if (lock) {
                memcpy(lock, text.c_str(), size);
                GlobalUnlock(hGlob);
                if (!SetClipboardData(CF_TEXT, hGlob)) {
                    GlobalFree(hGlob);
                }
            }
            else {
                GlobalFree(hGlob);
            }
        }
        CloseClipboard();
#else
        std::string command = "";
        if (command_exists("pbcopy")) {
            command = "pbcopy";
        }
        else if (command_exists("wl-copy")) {
            command = "wl-copy";
        }
        else if (command_exists("xclip")) {
            command = "xclip -selection clipboard";
        }
        else if (command_exists("xsel")) {
            command = "xsel --clipboard --input";
        }

        if (!command.empty()) {
            FILE* pipe = popen(command.c_str(), "w");
            if (pipe) {
                fwrite(text.c_str(), 1, text.size(), pipe);
                pclose(pipe);
            }
        }
        else {
            debug_log("Error: No supported clipboard manager found (xclip, xsel, wl-copy, pbcopy).");
        }
#endif
    }
    inline void blop(int x, int y) {
#ifdef _WIN32
        std::thread([=]() {
            Beep(x, y);
            }).detach();
#endif
    }
    struct event {
        Input_Type input;
        int mouse_x;
        int mouse_y;
    };
    inline event get_event() {
        event returner;
        returner.input = Get_Input();
        returner.mouse_x = get_MouseX();
        returner.mouse_y = get_MouseY();
        return returner;
    }
    inline event get_event_nonblocking() {
        event returner;
        returner.input = Get_Input_Nonblocking();
        returner.mouse_x = get_MouseX();
        returner.mouse_y = get_MouseY();
        return returner;
    }
}