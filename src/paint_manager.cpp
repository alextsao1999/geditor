//
// Created by Alex on 2019/6/26.
//

#include "document.h"
Painter::Painter(HDC m_HDC, EventContext *context) : m_HDC(m_HDC), m_context(context) {
    m_offset = context->current()->getOffset() - context->getPaintManager()->getViewportOffset();
}

