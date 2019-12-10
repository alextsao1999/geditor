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

#define ANSI

#ifdef UNICODE
#define GChar _TCHAR
#define _GT(t) _T(t)
#define gstrcmp wcscmp
#define gstrcat wcscat
#define gsprintf swprintf
#define gstrlen wcslen
#define gstrcpy wcscpy
#elif (defined(ANSI))
#define GChar char
#define _GT(t) t
#define gstrcmp strcmp
#define gstrcat strcat
#define gsprintf sprintf
#define gstrlen strlen
#define gstrcpy strcpy

#endif

#include "stdio.h"
#define NOT_REACHED() do { \
    fprintf(stderr, "shouldn't be reached! %s:%d", __FILE__, __LINE__); \
} while (0);

#define ASSERT(cond, msg) do { \
if (!(cond)) { \
    fprintf(stderr, "assert fail! \"%s\" %s:%d", msg, __FILE__, __LINE__); \
    abort(); \
} \
} while (0)


#endif //GEDITOR_COMMON_H
