//
// Created by Alex on 2019/6/25.
//

#include "document.h"

Offset Element::getOffset() {
    Offset offset = getLogicOffset();
    if (m_parent != nullptr) {
        Offset base = m_parent->getOffset();
        offset.x += base.x;
        offset.y += base.y;
    }
    return offset;
}

int Element::getLineNumber() {
    if (!hasChild())
        return 0;
    if (getDisplay() == Display::Line)
        return 1;
    int line = 0;
    for (auto ele : *children()) {
        switch (ele->getDisplay()) {
            case Display::Line:
                line++;
                break;
            case Display::Block:
                line += ele->getLineNumber();
            default:
                break;
        }
    }
    return line;
}

LineViewer EventContext::getLineViewer() {
    if (outer) {
        return outer->getLineViewer();
    } else {
        return doc->getContext()->m_textBuffer.getLine(line);
    }
}

void EventContext::set(Root *obj, int idx = 0) {
    if (obj->hasChild()) {
        buffer = obj->children();
        index = idx;
    }
}

Painter EventContext::getPainter() {
    return doc->getContext()->m_paintManager->getPainter(this);
}

EventContext EventContext::enter(Root *element, int idx) {
    return EventContext(doc, element->children(), this, idx);
}

EventContext EventContext::enter() {
    return EventContext(doc, current()->children(), this, 0);
}

PaintManager *EventContext::getPaintManager() {
    return doc->getContext()->m_paintManager;
}

LayoutManager *EventContext::getLayoutManager() {
    return &doc->getContext()->m_layoutManager;
}

CaretManager *EventContext::getCaretManager() {
    return &doc->getContext()->m_caretManager;
}

bool EventContext::prev() {
    if (index > 0) {
        index--;
        if (current()->getDisplay() == Display::Line) {
            prevLine();
        }
        return true;
    }
    return false;
}

bool EventContext::next() {
    if (current()->getDisplay() == Display::Line) {
        nextLine();
    }
    index++;
    return index < buffer->size();
}

void EventContext::reflow() {
    doc->m_context.m_layoutManager.reflow(*this);
}

void EventContext::focus() {
    doc->m_context.m_caretManager.focus(this);
}

void EventContext::combine() {
    int next = index + 1;
    if (next >= buffer->size()) {
        return;
    }
    auto &text = doc->getContext()->m_textBuffer;
    auto ele = buffer->at(next);
    if (current()->getDisplay() == Display::Line && ele->getDisplay() == Display::Line) {
        text.getLine(line).getContent() += text.getLine(line + 1).getContent();
        buffer->erase(next);
        doc->getContext()->m_textBuffer.deleteLine(line + 1);
        delete ele;
    }
}

void EventContext::redraw() {
    // redraw 需要更新宽度!!!
    doc->getContext()->m_paintManager->refresh();
}
LineViewer EventContext::copyLine() {
    if (current()->getDisplay() == Display::Line) {
        int next = getLine() + 1;
        Element *element = current()->copy();
        if (!element) {
            return {};
        }
        buffer->insert(index + 1, element);
        return doc->getContext()->m_textBuffer.insertLine(next);
    }
    return {};
}
void EventContext::push(CommandType type, CommandData data) {
    doc->getContext()->m_queue.push({copy(), type, data});
}
void EventContext::reflowBrother() {
    doc->getContext()->m_layoutManager.reflowEnter(*this);
}

Root *Root::getContain(EventContext &context, int x, int y) {
    if (!hasChild()) {
        return this;
    }
    for (auto ele : *children()) {
        if (ele->contain(context, x, y)) {
            return ele->getContain(context, x, y);
        }
    }
    return nullptr;
}
 EventContext Root::getContainEventContext(EventContext &context, int x, int y) {
     EventContext event = context.enter(this, 0);
     while (event.has()) {
         if (event.current()->contain(event, x, y)) {
             return event;
         }
         event.next();
     }
     return {};
}
void Root::redraw(EventContext &context) {
    EventContext ctx = context.enter();
    while (ctx.has()) {
        ctx.current()->redraw(ctx);
        ctx.next();
    }
}
void Root::reflow(EventContext &context) {
    context.reflow();
}

/*
EventContext Root::getChildEventContext(EventContext &context, int x, int y) {
    Offset offset = context.current()->getOffset();
    EventContext event = context.enter(this, 0);
    while (event.has()) {
        if (event.current()->contain(event, offset.x + x, offset.y + y)) {
            return event;
        }
        event.next();
    }
    return {};
}

Offset Root::getChildOffset(EventContext &context, int x, int y) {
    Offset offset(x, y);
    offset += getOffset() - context.current()->getOffset();
    return offset;
}
*/
