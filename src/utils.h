//
// Created by Administrator on 2019/9/2.
//

#ifndef GEDITOR_UTILS_H
#define GEDITOR_UTILS_H

#include "common.h"

void GSleep(double d);
void DoEvents(HWND hWnd = nullptr);
std::wstring AnsiToUnicode(const char *str);
std::string UnicodeToAnsi(const wchar_t *str);
#define A2W(ansi) (AnsiToUnicode((const char *)(ansi)).c_str())
#define W2A(unicode) (UnicodeToAnsi((const wchar_t *)(unicode)).c_str())

#endif //GEDITOR_UTILS_H
