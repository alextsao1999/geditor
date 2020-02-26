//
// Created by Alex on 2020/2/26.
//

#include "margin.h"
#include "document.h"

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
    return 2;
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
    GString &&string = std::to_wstring(context->getCounter().line + 1);
    auto style = context->getStyle().paint();
    //style.setColor(SkColorSetRGB(69,145,245));
    style.setColor(SK_ColorLTGRAY);
    Offset view = context->viewportOffset();
    view.x = m_lineWidth - 5;
    view.y += style.getTextSize() + 8;
    style.setTextAlign(SkPaint::Align::kRight_Align);
    if (auto *render = (WindowRenderManager *) m_doc->m_context.m_renderManager) {
        render->m_canvas->drawText(string.c_str(), string.size() * 2, view.x, view.y, style);
    }

}

