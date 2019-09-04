//
// Created by Administrator on 2019/9/3.
//

#include "caret_manager.h"
#include "document.h"

void CaretManager::focus(EventContext *context) {
    if (m_context) {
        m_focus->onBlur(*m_context);
        m_context->free();
        delete m_context;
    }
    m_context = context->copy();
    m_focus = context->current();
    m_focus->onFocus(*m_context);
}

CaretManager::~CaretManager() {
    if (m_context) {
        m_context->free();
        delete m_context;
    }
}

void CaretManager::update() {
    if (!m_focus)
        return;
    Offset offset = m_focus->getOffset() + m_current - m_paintManager->getViewportOffset();
    SetCaretPos(offset.x, offset.y);
}
