//
// Created by Alex on 2019/6/26.
//

#include "document.h"
Painter::Painter(HDC m_HDC, EventContext *context) : m_HDC(m_HDC), m_context(context) {
    m_offset = context->current()->getOffset(*context) - context->getRenderManager()->getViewportOffset();
}

Canvas::Canvas(SkCanvas *mCanvas, EventContext *context) : m_canvas(mCanvas) {
    m_offset = context->current()->getOffset(*context) - context->getRenderManager()->getViewportOffset();
//    SkRect rect{};
//    rect.setXYWH(m_offset.x, m_offset.y, CallEvent(*context, getWidth), CallEvent(*context, getHeight));
    //m_canvas->saveLayer(&rect, nullptr);
    m_canvas->translate(SkIntToScalar(m_offset.x), SkIntToScalar(m_offset.y));
}

Canvas::~Canvas() {
    //m_canvas->restore();
    m_canvas->translate(SkIntToScalar(-m_offset.x), SkIntToScalar(-m_offset.y));
}

