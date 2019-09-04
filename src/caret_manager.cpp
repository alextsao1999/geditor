//
// Created by Administrator on 2019/9/3.
//

#include "caret_manager.h"
#include "document.h"

void CaretManager::focus(EventContext *context) {
    Element *focus = getFocus();
    if (m_context) {
        focus->onBlur(*m_context);
        m_context->free();
        delete m_context;
    }
    m_context = context->copy();
    focus = context->current();
    focus->onFocus(*m_context);
}

CaretManager::~CaretManager() {
    if (m_context) {
        m_context->free();
        delete m_context;
    }
}

void CaretManager::update() {
    if (!getFocus())
        return;
    Offset offset = getFocus()->getOffset() + m_current - m_paintManager->getViewportOffset();
    SetCaretPos(offset.x, offset.y);
}

Element *CaretManager::getFocus() {
    if (!m_context) {
        return nullptr;
    }
    return m_context->current();
}
