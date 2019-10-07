//
// Created by Alex on 2019/9/2.
//

#include "utils.h"

void GSleep(double d) {
    LARGE_INTEGER time;
    double dDlay = 0;
    QueryPerformanceCounter(&time);
    LONGLONG start = time.QuadPart;
    while (dDlay - d < 0.1) {
        QueryPerformanceCounter(&time);
        dDlay = (time.QuadPart - start);
    }
}

void DoEvents(HWND hWnd) {
    MSG msg;
    BOOL result;
    while ( ::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE ) )
    {
        result = ::GetMessage(&msg, NULL, 0, 0);
        if (result == 0) // WM_QUIT
        {
            ::PostQuitMessage(msg.wParam);
            break;
        }
        else
        {
            ::TranslateMessage(&msg);
            :: DispatchMessage(&msg);
        }
    }
}
