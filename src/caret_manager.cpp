//
// Created by Alex on 2019/9/3.
//

#include "caret_manager.h"
#include "document.h"

void CaretManager::focus(EventContext *context, bool force) {
    EventContext *before = m_context;
    if (before) {
        if (before->element) {
            if (before == context) {// 元素相同 FIXME 可能有问题
                before->element->onBlur(*before, before);
                before->element->onFocus(*before);
                return;
            }
            if (!force) {
                before->element->onBlur(*before, context);
            }
        }
        before->free();
    }
    m_context = context;
    if (context) {
        context_on(*context, Focus);
        // 更新光标位置
        update();
    }
}

CaretManager::~CaretManager() {
    if (m_context) {
        m_context->free();
    }
}

void CaretManager::update() {
    if (!m_context) {
        return;
    }
    // 设置光标的位置为实际偏移(光标偏移 减去 可视区偏移)
    Offset offset = current() - m_paintManager->getViewportOffset();
    SetCaretPos(offset.x, offset.y);
}

Element *CaretManager::getFocus() {
    return m_context ? m_context->current() : nullptr;
}

bool CaretManager::enter(int index) {
    if (m_context && m_context->canEnter()) {
        m_context = new EventContext(m_context, index);
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
        m_context->next();
        if (!m_context->has()) {
            *m_context = cur;
            return false;
        }
        cur.current()->onBlur(cur, m_context);
        m_context->current()->onFocus(*m_context);
        return true;
    }
    return false;
}

bool CaretManager::prev() {
    if (m_context) {
        EventContext last = *m_context;
        m_context->prev();
        if (!m_context->has()) {
            *m_context = last;
            return false;
        }
        last.current()->onBlur(last, m_context);
        m_context->current()->onFocus(*m_context);
        return true;

    }
    return false;
}

Offset CaretManager::current() {
    if (m_context) {
        return m_context->offset() - m_context->caretOffset() + m_current;
    }
    return m_current;
}

bool CaretManager::findNext(const GChar *tag) {
    EventContext *next = m_context->findNext(tag);
    if (next != nullptr) {
        focus(next);
        return true;
    }
    m_data.setIndex(-1);
    focus(m_context);
    return false;
}

bool CaretManager::findPrev(const GChar *tag) {
    EventContext *prev = m_context->findPrev(tag);
    if (prev != nullptr) {
        focus(prev);
        return true;
    }
    m_data.setIndex(0);
    focus(m_context);
    return false;
}

EventContext *CaretManager::include(Element *element) {
    if (!m_context) {
        return nullptr;
    }
    return m_context->include(element);
}
