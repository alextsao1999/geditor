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
std::wstring AnsiToUnicode(const char *str) {
    size_t alength = strlen(str);
    size_t len = MultiByteToWideChar(0, 0, str, alength, 0, 0);
    std::wstring gstr;
    if (len > 0) {
        gstr.resize(len);
        MultiByteToWideChar(0, 0, str, alength, &gstr.front(), len);
    }
    return gstr;
}
std::string UnicodeToAnsi(const wchar_t *str) {
    size_t llen = wcslen(str);
    size_t len = WideCharToMultiByte(0, 0, str, llen, 0, 0, 0, 0);
    std::string gstr;
    if (len > 0) {
        gstr.resize(len);
        WideCharToMultiByte(0, 0, str, llen, (LPSTR) &gstr.front(), gstr.size(), 0, 0);
    }
    return gstr;
}
