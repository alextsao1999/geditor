//
// Created by Alex on 2019/9/3.
//

#include "caret_manager.h"
#include "document.h"

void EventContextRef::unref() {
    if (m_ref) {
        if ((*m_ref)-- <= 0) {
            m_ptr->free();
            delete m_ref;
        }
    }
}

CaretManager::~CaretManager() {
/*
    if (m_context.has()) {
        m_context->free();
    }
*/
}

void CaretManager::focus(EventContext *sender, EventContext *context, bool force) {
    EventContextRef before = m_context;
    m_context = context;
    if (before.has()) {
        if (before->element) {
            before->element->onBlur(*before, context, force);
        }
    }
    if (m_context.ptr() != context) {
        return;
    }
    if (m_context.has()) {
        Size size = m_paintManager->getViewportSize();
        Offset view = m_context->viewportOffset();
        if (view.y < 0) {
            Offset offset = m_context->offset();
            offset.x = 0;
            m_paintManager->setViewportOffset(offset);
        }
        view.y += m_context->height();
        if (view.y > size.height) {
            Offset offset = m_context->offset();
            offset.x = 0;
            offset.y = offset.y + m_context->height() - size.height;
            m_paintManager->setViewportOffset(offset);
        }
        context_on(*m_context, Focus);
        // 更新光标位置
        update();
    }
}

void CaretManager::set(Offset pos, int index) {
    if (!m_context) {
        return;
    }
    m_data.set(index, m_context->absOffset(pos));
    m_relative = pos;
    update();
}

void CaretManager::update() {
    // 设置光标的位置为实际偏移(光标偏移 减去 可视区偏移)
    Offset offset = current();
    SetCaretPos(offset.x, offset.y);
    onShow();
}

Offset CaretManager::current() {
    if (!m_context) {
        return m_relative;
    }
    return m_context->offset() - m_context->caretOffset() + m_relative - m_paintManager->getViewportOffset();
}

bool CaretManager::enter(int index) {
    if (m_context && m_context->canEnter()) {
        m_context.m_ptr = new EventContext(m_context.ptr(), index);
        return true;
    }
    return false;
}

void CaretManager::leave() {
    EventContext *outer = m_context->outer;
    delete m_context.m_ptr;
    m_context.m_ptr = outer;
}

bool CaretManager::next() {
    if (m_context.has()) {
        EventContext cur = *m_context;
        m_context->next();
        if (!m_context->has()) {
            *m_context = cur;
            return false;
        }
        refocus();
        return true;
    }
    return false;
}

bool CaretManager::prev() {
    if (m_context.has()) {
        EventContext last = *m_context;
        m_context->prev();
        if (!m_context->has()) {
            *m_context = last;
            return false;
        }
        refocus();
        return true;
    }
    return false;
}

bool CaretManager::findNext(char *tag) {
    EventContext *next = m_context->findNext(tag);
    if (next != nullptr) {
        focus(m_context.ptr(), next);
        return true;
    }
    m_data.setIndex(-1);
    refocus();
    return false;
}

bool CaretManager::findPrev(char *tag) {
    EventContext *prev = m_context->findPrev(tag);
    if (prev != nullptr) {
        focus(m_context.ptr(), prev);
        return true;
    }
    m_data.setIndex(0);
    refocus();
    return false;
}

EventContext *CaretManager::include(Element *element) {
    if (!m_context.has()) {
        return nullptr;
    }
    return m_context->include(element);
}

EventContext *CaretManager::include(EventContext *context) {
    if (!m_context.has()) {
        return nullptr;
    }
    return m_context->include(context);
}

void CaretManager::onErase(EventContext *context) {
    if (m_context.has() && context) {
        if (m_context->outer) {
            EventContext *ctx = m_context->outer->include(context->outer);
            EventContext *find = m_context.ptr();
            while (find && find->outer != ctx) {
                find = find->outer;
            }
            if (find && find->index > context->index) {
                find->index--;
                find->prevLine(context->current()->getLineNumber());
            }
        }
    }
}

void CaretManager::onTick() {
    if (hide) {
        return;
    }
    constexpr double step = 0.05f * 2.3;
    if (direct) {
        process += step;
    } else {
        process -= step;
    }
    alpha = int(EaseInOutQuint(process) * 255);
    if (alpha >= 255) {
        direct = false;
        alpha = 255;
    }
    if (alpha <= 0) {
        direct = true;
        alpha = 0;
    }
    m_paintManager->refresh();
}
void CaretManager::onDraw() {
    if (hide || !m_context) {
        return;
    }
    if (auto *render = (WindowRenderManager *) m_paintManager) {
        Offset offset = current();
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        paint.setStrokeWidth(2);
        paint.setAlpha(alpha);
        int height = 17;
        render->m_canvas->drawLine(offset.x, offset.y, offset.x, offset.y + height, paint);
    }

}
void CaretManager::onShow() {
    hide = false;
    process = 0.5;
    direct = true;
    onTick();
}
