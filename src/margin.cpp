//
// Created by Alex on 2020/2/26.
//

#include "margin.h"
#include "document.h"
#include "table.h"
Margin::Margin(Document *doc) : m_doc(doc) {
    m_charWidth = doc->m_root.getStyle().measureText(_GT("0"), sizeof(GChar));
    m_lineWidth = m_charWidth + 15;
}

void Margin::update() {
    int count = m_doc->context()->m_textBuffer.getLineCount();
    m_lineWidth = (int(std::log10(count)) + 1) * m_charWidth + 15;

}

int Margin::index(Offset offset) {
    if (offset.x > width()) {
        return 0;
    }
    if (offset.x < m_lineWidth) {
        return 1;
    }
    int index = 1;
    int current = m_lineWidth;
    for (int width : m_widths) {
        if (offset.x >= current) {
            index++;
        }
        current += width;
    }
    return index;
}

void Margin::draw() {
    if (auto *render = (WindowRenderManager *) m_doc->context()->m_renderManager) {
        GScalar height = SkIntToScalar(render->getViewportSize().height);
        SkPaint paint;
        paint.setColor(SK_ColorLTGRAY);
        render->m_canvas->drawLine(m_lineWidth, 0, m_lineWidth, height, paint);
    }
}

void Margin::drawGutter(EventContext *context) {
    GString &&string = std::to_wstring(context->line() + 1);
    auto style = context->getStyle().paint();
    //style.setColor(SkColorSetRGB(69,145,245));
    style.setColor(SK_ColorLTGRAY);
    style.setTextAlign(SkPaint::Align::kRight_Align);
    if (auto *render = (WindowRenderManager *) m_doc->m_context.m_renderManager) {
        Offset view = context->viewportOffset();
        view.x = m_lineWidth - 5;
        render->m_canvas->drawText(string.c_str(), string.size() * 2,
                                   view.x, (float) view.y + style.getTextSize() + 8, style);
        int flags = context->getLineViewer().flags();
        if (flags & LineFlagBP) {
            render->m_canvas->drawCircle(offset(2) + 8, view.y + 16, 3, style);
        }
        if ((flags & LineFlagFold) || (flags & LineFlagExpand)) {
            style.setStyle(SkPaint::kStroke_Style);
            float lineRight = SkIntToScalar(offset(3)) + 6.5f;
            float lineTop = SkIntToScalar(view.y) + 10.5f;
            GPath path;
            path.addRect(GRect::MakeXYWH(lineRight, lineTop, 10, 10));
            SkPoint pts[] = {{lineRight + 2, lineTop + 5.0f},
                             {lineRight + 8, lineTop + 5.0f}};
            path.addPoly(pts, SK_ARRAY_COUNT(pts), false);
            if (flags & LineFlagFold) {
                pts[0] = {lineRight + 5, lineTop + 10.0f};
                pts[1] = {lineRight + 5, lineTop - 8.0f + (float) context->height()};
                path.addPoly(pts, SK_ARRAY_COUNT(pts), false);
            } else {
                pts[0] = {lineRight + 5, lineTop + 2};
                pts[1] = {lineRight + 5, lineTop + 8};
                path.addPoly(pts, SK_ARRAY_COUNT(pts), false);
            }
            if (flags & LineFlagLineVert) {
                pts[0] = {lineRight + 5, lineTop - 10.5f};
                pts[1] = {lineRight + 5, lineTop};
                path.addPoly(pts, SK_ARRAY_COUNT(pts), false);
            }
            render->m_canvas->drawPath(path, style);
        }
        if ((flags & LineFlagLineVert) && !(flags & LineFlagFold)) {
            float lineRight = SkIntToScalar(offset(3)) + 11.5f;
            float lineTop = SkIntToScalar(view.y);
            SkPoint pts[] = {{lineRight, lineTop},
                             {lineRight, lineTop + (float) context->height()}};
            render->m_canvas->drawPoints(SkCanvas::kLines_PointMode, 2, pts, style);
        }
        if (flags & LineFlagLineHorz) {
            float lineRight = SkIntToScalar(offset(3)) + 11.5f;
            float lineTop = SkIntToScalar(view.y);
            SkPoint pts[] = {{lineRight,        lineTop},
                             {lineRight,        lineTop + 15.5f},
                             {lineRight + 6.0f, lineTop + 15.5f}};
            render->m_canvas->drawPoints(SkCanvas::kPolygon_PointMode, 3, pts, style);
        }

    }

}

