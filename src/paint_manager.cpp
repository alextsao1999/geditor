//
// Created by Alex on 2019/6/26.
//

#include "document.h"
#include "geditor.h"
#include "paint_manager.h"
#include "SkTextBlob.h"
#include "SkSurface.h"
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
/*
    SkTextBlobBuilder builder;
    SkPaint paint = m_context->getStyle(style).paint();
    paint.setTextEncoding(SkPaint::TextEncoding::kGlyphID_TextEncoding);
    int count = paint.countText(text, byteLength);
    if (count <= 0) {
        return;
    }
    auto &run = builder.allocRun(paint, count, 0, 0);
    m_context->getStyle(style).paint().textToGlyphs(text, byteLength, run.glyphs);
    m_canvas->drawTextBlob(builder.build(), x, y, paint);
*/

    if (m_context) {
        return;
        if (m_context->selecting()) {
            GStyle &paint = m_context->getStyle(StyleSelectedFont);
            auto start = m_context->getDocContext()->m_selectStartPos;
            auto end = m_context->getDocContext()->m_selectEndPos;
            m_canvas->drawText((GChar *) text + start.index, start.index - end.index, start.offset.x, start.offset.y,
                               paint.paint());


/*
            printf("[%d %d] start %d, end %d pos: %d %d\n", m_context->isSelectedStart(), m_context->isSelectedEnd(),
                   start.index,
                   end.index, start.offset.x, end.offset.x);
*/

            //m_canvas->drawText(text, byteLength, x, y, paint.paint());

        }
    }


}

void Canvas::drawRect(const GRect &rect, int style) {
    m_canvas->drawRect(rect, m_context->getStyle(style).paint());
}

void RenderManager::redraw(EventContext *ctx) {
    Offset offset = ctx->viewportOffset();
    RECT rect;
    rect.left = offset.x;
    rect.top = offset.y;
    rect.right = rect.left + ctx->width();
    rect.bottom = rect.top + ctx->height();
    //InvalidateRect(m_hWnd, &rect, false);
    InvalidateRect(m_hWnd, nullptr, false);
}

void RenderManager::redrawRect(GRect *rect) {
    update();
//    m_data->m_document.onDraw(m_data->m_document.m_root, m_canvas.get());
    m_data->m_document.onRedraw(m_data->m_document.m_root);

/*
    if (context.isSelected()) {
        Canvas canvas = context.getCanvas();
        SkPaint color;
        color.setColor(SK_ColorCYAN);
        color.setAlpha(150);
        auto bound = context.relative(select.x(), select.y());
        canvas->drawRect(GRect::MakeXYWH(bound.x, bound.y, select.width(), select.height()), color);
    }
*/
}

void RenderManager::updateViewport(LayoutManager *layoutManager) {
    Offset offset;
    offset.x = GetScrollPos(m_hWnd, SB_HORZ);
    offset.y = GetScrollPos(m_hWnd, SB_VERT);
    m_data->m_document.setViewportOffset(offset);
    refresh();
}

Offset RenderManager::getViewportOffset() {
    return m_data->m_document.m_viewportOffset;
}



