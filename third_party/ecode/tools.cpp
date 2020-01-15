//
// Created by 曹顺 on 2019/5/4.
//

#include "tools.h"

char *GetLibPath() {
    HKEY key;
    if (RegOpenKeyA(HKEY_CURRENT_USER, R"(Software\FlySky\E\Install)", &key) == 0) {
        char buffer[255];
        DWORD dwType;
        DWORD length = 255;
        RegQueryValueExA(key, "Path", nullptr, &dwType, (LPBYTE) buffer, &length);
        char *ret = new char[length + 1];
        memcpy(ret, buffer, length);
        RegCloseKey(key);
        ret[length] = '\0';
        return ret;
    }
    return nullptr;
}
