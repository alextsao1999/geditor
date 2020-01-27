//
// Created by Alex on 2020/1/15.
//

#include "open_visitor.h"

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

