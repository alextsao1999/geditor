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
        buffer = obj->children()->getPointer();
        index = idx;
    }
}

Painter EventContext::getPainter() {
    return doc->getContext()->m_paintManager->getPainter(this);
}

EventContext EventContext::enter(Root *element, int index) {
    return EventContext(doc, element->children()->getPointer(), this, index);
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

void EventContext::next() {
    if (current()->getDisplay() == Display::Line) {
        nextLine();
    }
    index++;
}

void EventContext::reflow() {
    doc->m_context.m_layoutManager.reflow(*this);
}

void EventContext::focus() {
    doc->m_context.m_caretManager.focus(this);
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
