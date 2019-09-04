//
// Created by Alex on 2019/6/26.
//

#include "document.h"
Painter::Painter(HDC m_HDC, EventContext *context, ObjectManger *obj) : m_HDC(m_HDC), m_context(context), m_object(obj) {
    m_offset = context->current()->getOffset() - context->getPaintManager()->getViewportOffset();
    SelectObject(m_HDC, m_object->getFont(ObjectManger::FontDefault));

}

