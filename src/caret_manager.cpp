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
    Offset offset = m_current - m_paintManager->getViewportOffset();
    SetCaretPos(offset.x, offset.y);
}

Element *CaretManager::getFocus() {
    if (!m_context) {
        return nullptr;
    }
    return m_context->current();
}

void CaretManager::set(Offset pos) {
    m_current = m_context->offset() + pos;
    update();
}

void CaretManager::autoSet(int x, int y, int column) {
    auto meter = m_context->getRenderManager()->getTextMetrics();
    auto line = m_context->getLineViewer(column);
    int width = meter.measure(line.str(), m_data.index);
    set(x + width, y);
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

void CaretManager::next() {
    if (m_context) {
        m_context->current()->onBlur(*m_context);
        if (!m_context->next()) {
            m_context->prev();
        }
        m_context->current()->onFocus(*m_context);
    }
}

void CaretManager::prev() {
    if (m_context) {
        EventContext last = *m_context;
        if (m_context->prev()) {
            last.current()->onBlur(last);
            m_context->current()->onFocus(*m_context);
        }
    }
}

void CaretManager::outerNext() {

}

void CaretManager::outerPrev() {

}
