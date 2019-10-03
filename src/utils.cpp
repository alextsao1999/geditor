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

void DoEvents() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        DispatchMessage(&msg);
        TranslateMessage(&msg);
    }
}
