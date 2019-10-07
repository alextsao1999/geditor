//
// Created by Alex on 2019/6/26.
//

#include "document.h"
#include "paint_manager.h"

Painter::Painter(HDC m_HDC, EventContext *context) : m_HDC(m_HDC), m_context(context) {
    m_offset = context->offset() - context->getRenderManager()->getViewportOffset();
}

Canvas::Canvas(EventContext *context, SkCanvas *canvas, SkPaint *paint) : m_canvas(canvas), m_context(context) {
    m_offset = context->viewportOffset();
    //m_count = m_canvas->save();
    GRect rect = GRect::MakeXYWH(m_offset.x, m_offset.y, context->width() + 1, context->height() + 1);
    m_count = m_canvas->saveLayer(&rect, paint);
    m_canvas->translate(SkIntToScalar(m_offset.x), SkIntToScalar(m_offset.y));
}

Canvas::~Canvas() {
    m_canvas->restoreToCount(m_count);
}

SkRect Canvas::bound(Offset inset) {
    SkRect rect{
            0,
            0,
            SkIntToScalar(m_context->width()),
            SkIntToScalar(m_context->height())
    };

    rect.inset(inset.x, inset.y);
    return rect;
}

void RenderManager::redraw(EventContext *ctx) {
    Offset offset = ctx->offset() - getViewportOffset();
    RECT rect;
    rect.left = offset.x;
    rect.top = offset.y;
    rect.right = rect.left + ctx->width();
    rect.bottom = rect.top + ctx->height();
    InvalidateRect(m_hWnd, &rect, false);
}


