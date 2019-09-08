//
// Created by Alex on 2019/7/12.
//
#ifndef GEDITOR_MEMORY_H
#define GEDITOR_MEMORY_H

// 之后根据需要加装内存池...
#include <stdlib.h>
#define ge_malloc malloc
#define ge_free free
#define ge_realloc realloc

int CeilToPowerOf2(int v);

#endif //GEDITOR_MEMORY_H
