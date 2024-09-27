#include <stdio.h>
#include <windows.h>

#include <iostream>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

constexpr TCHAR FONT_NAME[] = TEXT("Hermit");

struct {
    HFONT hFont;
} static STATIC;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LPCWSTR text = (LPCWSTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    
    if (uMsg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        SelectObject(hdc, STATIC.hFont);

        SetTextColor(hdc, RGB(0, 0, 0));

        TextOutW(hdc, 0, 0, text, lstrlenW(text)); // Display text

        EndPaint(hwnd, &ps);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static DWORD CALLBACK DisplayTextThread(LPVOID data) {
    LPCWSTR text = (LPCWSTR)data;

    HINSTANCE hInstance = (HINSTANCE)&__ImageBase;

    TCHAR CLASS_NAME[] = TEXT("SampleWindowClass");

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        CLASS_NAME,                             // Window class
        NULL,                                   // Window text
        WS_POPUP,                               // No title bar or control buttons
        0, 0, 0, 0,                             // Size and position
        NULL,                                   // Parent window
        NULL,                                   // Menu
        hInstance,                              // Instance handle
        NULL                                    // Additional application data
    );

    if (hwnd == NULL) {
        exit(1);
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)text);

    if (!STATIC.hFont) {    
        STATIC.hFont = CreateFont(
            36,                  // Height of font
            0,                   // Width of font
            0,                   // Escapement
            0,                   // Orientation
            FW_BOLD,             // Weight
            FALSE,               // Italic
            FALSE,               // Underline
            FALSE,               // StrikeOut
            DEFAULT_CHARSET,     // Character set
            OUT_OUTLINE_PRECIS,  // Output precision
            CLIP_DEFAULT_PRECIS, // Clipping precision
            DEFAULT_QUALITY,     // Output quality
            FF_DONTCARE,         // Font family
            FONT_NAME            // Font name
        );
    }

    HDC hdc = GetDC(hwnd);
    SelectObject(hdc, STATIC.hFont);
    SIZE textSize;
    GetTextExtentPoint32W(hdc, text, lstrlenW(text), &textSize);
    ReleaseDC(hwnd, hdc);

    int width = textSize.cx;
    int height = textSize.cy;

    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, width, height, 0);
    SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_NOACTIVATE);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Create a timer to close the window after 3 seconds
    SetTimer(hwnd, 1, 1000, [](HWND, UINT, UINT_PTR, DWORD) -> VOID {
        PostQuitMessage(0);
    });

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void DisplayText(LPCWSTR text) {
    CreateThread(NULL, 0, DisplayTextThread, (LPVOID)text, 0, NULL);
}
