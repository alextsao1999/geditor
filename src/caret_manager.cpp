//
// Created by Alex on 2019/9/3.
//

#include "caret_manager.h"
#include "document.h"

void CaretManager::focus(EventContext *context) {
    Element *focus = getFocus();
    if (focus) {
        if (focus == context->current())
            return;
        focus->onBlur(*m_context);
        m_context->free();
    }
    m_context = context->copy();
    focus = context->current();
    focus->onFocus(*m_context);
}

CaretManager::~CaretManager() {
    if (m_context) {
        m_context->free();
    }
}

void CaretManager::update() {
    if (!getFocus())
        return;
    // 设置光标的位置为实际偏移(光标偏移 减去 可视区偏移)
    Offset offset = m_current - m_paintManager->getViewportOffset();
    SetCaretPos(offset.x, offset.y);
}

Element *CaretManager::getFocus() {
    return m_context ? m_context->current() : nullptr;
}

void CaretManager::set(Offset pos) {
    m_current = m_context->offset() + pos;
    update();
}

bool CaretManager::enter(int index) {
    if (m_context && m_context->current()->hasChild()) {
        m_context = new EventContext(m_context->doc, m_context->current()->children(), m_context, index);
        return true;
    }
    return false;
}

void CaretManager::leave() {
    EventContext *outer = m_context->outer;
    delete m_context;
    m_context = outer;
}

bool CaretManager::next() {
    if (m_context) {
        EventContext cur = *m_context;
        if (!m_context->next()) {
            m_context->prev();
            return false;
        }
        cur.current()->onBlur(cur);
        m_context->current()->onFocus(*m_context);
        return true;
    }
    return false;
}

bool CaretManager::prev() {
    if (m_context) {
        EventContext last = *m_context;
        if (m_context->prev()) {
            last.current()->onBlur(last);
            m_context->current()->onFocus(*m_context);
            return true;
        }
    }
    return false;
}

void CaretManager::outerNext() {

}

void CaretManager::outerPrev() {

}
