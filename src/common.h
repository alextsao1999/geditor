//
// Created by Administrator on 2019/6/28.
//

#ifndef GEDITOR_COMMON_H
#define GEDITOR_COMMON_H

#include <Windows.h>
#include <wingdi.h>
#include "WinUser.h"
#include <tchar.h>
#include <string>
#include "memory.h"

struct EventContext;

#define GUNICODE

#ifdef GUNICODE
#define ToGS(t) A2W(t)
#define GString std::wstring
#define ToGString std::to_wstring
#define GChar wchar_t
#define GIStringStream std::wistringstream
#define _GT(t) L##t
#define gstrcmp wcscmp
#define gstrcat wcscat_s
#define gsprintf swprintf
#define gstrlen wcslen
#define gstrcpy wcscpy
#define gstrchr wcschr
#define gmemcpy wmemcpy
#define gmemcmp wmemcmp
#elif (defined(GANSI))
#define ToGS(t) t
#define ToGString std::to_string
#define GString std::string
#define GChar char
#define GIStringStream std::istringstream
#define _GT(t) t
#define gstrcmp strcmp
#define gstrcat strcat
#define gsprintf sprintf
#define gstrlen strlen
#define gstrcpy strcpy
#define gstrchr strchr
#define gmemcpy memcpy
#define gmemcmp memcmp

#endif

#include <cstdio>

#define GASSERT(cond, msg) do { \
if (!(cond)) { \
    fprintf(stderr, "assert fail! \"%s\" %s:%d", msg, __FILE__, __LINE__); \
    abort(); \
} \
} while (0)


#endif //GEDITOR_COMMON_H
