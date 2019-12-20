//
// Created by Alex on 2019/6/26.
//

#include "document.h"
#include "geditor.h"
#include "paint_manager.h"

Painter::Painter(HDC m_HDC, EventContext *context) : m_HDC(m_HDC), m_context(context) {
    m_offset = context->offset() - context->getRenderManager()->getViewportOffset();
}

void Painter::drawText(const void *text, size_t byteLength, GScalar x, GScalar y, int style) {
    GStyle gstyle = m_context->getStyle(style);
    gstyle.attach(m_HDC);
    SetTextColor(m_HDC, gstyle.paint().getColor() << 8);
    SetBkMode(m_HDC, TRANSPARENT);
    TextOut(m_HDC, m_offset.x + x, m_offset.y + y, (const GChar *) text, gstyle.countText(text, byteLength));
}

Canvas::Canvas(EventContext *context, SkCanvas *canvas) : m_canvas(canvas), m_context(context) {
    m_offset = m_context->viewportOffset();
    save();
}

Canvas::Canvas(EventContext *context, SkCanvas *canvas, SkPaint *paint) : m_canvas(canvas), m_context(context) {
    m_offset = m_context->viewportOffset();
    save(paint);
}

Canvas::~Canvas() {
    restore();
    delete m_canvas;
}

void Canvas::save() {
    m_count = m_canvas->save();
    m_canvas->translate(SkIntToScalar(m_offset.x), SkIntToScalar(m_offset.y));
}

void Canvas::save(SkPaint *paint) {
    GRect rect = GRect::MakeXYWH(m_offset.x, m_offset.y, m_context->width() + 1, m_context->height() + 1);
    m_count = m_canvas->saveLayer(&rect, paint);
    m_canvas->translate(SkIntToScalar(m_offset.x), SkIntToScalar(m_offset.y));
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

GRect Canvas::bound(SkScalar dx, SkScalar dy) {
    SkRect rect{
            0,
            0,
            SkIntToScalar(m_context->width() + 1),
            SkIntToScalar(m_context->height() + 1)
    };

    rect.inset(dx, dy);
    return rect;
}

void Canvas::drawText(const void *text, size_t byteLength, GScalar x, GScalar y, int style) {
    m_canvas->drawText(text, byteLength, x, y, m_context->getStyle(style).paint());
}

void Canvas::drawRect(const GRect &rect, int style) {
    m_canvas->drawRect(rect, m_context->getStyle(style).paint());
}

void RenderManager::redraw(EventContext *ctx) {
    Offset offset = ctx->offset() - getViewportOffset();
    RECT rect;
    rect.left = offset.x;
    rect.top = offset.y;
    rect.right = rect.left + ctx->width();
    rect.bottom = rect.top + ctx->height();
    //InvalidateRect(m_hWnd, &rect, false);
    InvalidateRect(m_hWnd, nullptr, false);
}

void RenderManager::redrawRect(GRect *rect) {
    if (m_data->m_begin.empty()) {
        m_data->m_begin = m_data->m_document.m_root.enter();
    }
    EventContext context = m_data->m_begin;
    GRect select = m_data->m_document.getContext()->getSelectRect();
    update();
    while (context.has()) {
        if (context.display() != DisplayNone) {
            context.current()->onRedraw(context);
            if (context.selected()) {
                Canvas canvas = context.getCanvas();
                SkPaint color;
                color.setColor(SK_ColorCYAN);
                color.setAlpha(150);
                auto bound = context.relative(select.x(), select.y());
                canvas->drawRect(GRect::MakeXYWH(bound.x, bound.y, select.width(), select.height()), color);
            }
        }
        context.next();
    }
/*
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::Style::kStroke_Style);
    paint.setStrokeWidth(1);
    rect->inset(0.5, 0.5);
    m_canvas->drawRect(*rect, paint);
*/

}

void RenderManager::updateBegin(Offset before, Offset now) {
    if (before.y > now.y) { // 向上
        while (m_data->m_begin.has()) {
            if (!m_data->m_begin.visible()) {
                break;
            }
            m_data->m_begin.prev();
        }
    } else { // 向下
        while (m_data->m_begin.has()) {
            if (m_data->m_begin.visible()) {
                break;
            }
            m_data->m_begin.next();
        }
    }
    if (m_data->m_begin.empty()) {
        m_data->m_begin = m_data->m_document.m_root.enter();
    }
}



