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


LineViewer EventContext::getLineViewer() {
    if (outer) {
        return outer->getLineViewer();
    } else {
        return doc->getContext()->m_textBuffer.getLine(line);
    }
}

void EventContext::set(Root *obj, int idx = 0) {
    if (obj->getChildrenNum()) {
        buffer = obj->children()->getPointer();
        index = idx;
    }
}

Painter EventContext::getPainter() {
    return doc->getContext()->m_paintManager->getPainter(this);
}

EventContext EventContext::enter(Element *element, int index) {
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
    if (current()->getDisplay() == Display::Block) {
        nextLine();
    }
    index++;
}

Root *Root::getContain(EventContext &context, int x, int y) {
    if (!getChildrenNum()) {
        return this;
    }
    for (auto ele : *children()) {
        if (ele->contain(context, x, y)) {
            return ele->getContain(context, x, y);
        }
    }
    return nullptr;
}
