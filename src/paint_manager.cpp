//
// Created by Alex on 2019/6/26.
//

#include "document.h"
GDIPainter::GDIPainter(HDC m_HDC, EventContext *context, PaintManger *obj) : m_HDC(m_HDC), m_context(context), m_object(obj) {
    m_offset = context->current()->getOffset(*context) - context->getPaintManager()->getViewportOffset();

}

