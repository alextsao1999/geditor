//
// Created by Alex on 2019/7/12.
//

#include "memory.h"

int CeilToPowerOf2(int v) {
    v += (v == 0);
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

