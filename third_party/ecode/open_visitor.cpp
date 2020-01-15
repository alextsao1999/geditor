//
// Created by Alex on 2020/1/15.
//

#include "open_visitor.h"

std::wstring AnsiToUnicode(const char *str) {
    size_t alength = strlen(str);
    size_t len = MultiByteToWideChar(0, 0, str, alength, 0, 0);
    std::wstring gstr;
    if (len > 0) {
        gstr.resize(len * 2);
        MultiByteToWideChar(0, 0, str, alength, &gstr.front(), gstr.size());
    }
    return gstr;
}

