//
// Created by ²ÜË³ on 2019/5/3.
//

#include "FileBuffer.h"
#include "windows.h"
std::wstring FixedData::toUnicode() {
    size_t len = MultiByteToWideChar(0, 0, data, length, 0, 0);
    std::wstring gstr;
    if (len > 0) {
        gstr.resize(len * 2);
        MultiByteToWideChar(0, 0, data, length, &gstr.front(), gstr.size());
    }
    return gstr;
}

std::string FixedData::toUtf8() {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> wct;
    return wct.to_bytes(toUnicode());
}
