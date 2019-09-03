//
// Created by Alex on 2019/6/26.
//

#include "document.h"
Painter::Painter(HDC m_HDC, EventContext *context) : m_HDC(m_HDC), context(context) {
    offset = context->current()->getOffset();

}
