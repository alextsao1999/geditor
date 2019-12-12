//
// Created by Alex on 2019/6/25.
//

#include "document.h"

Offset Element::getOffset(EventContext &context) {
    Offset offset = getLogicOffset();
    if (context.outer) {
        Offset base = CallEvent(*context.outer, getOffset);
        offset.x += base.x;
        offset.y += base.y;
    }
    return offset;
}

int Element::getLineNumber() {
    if (getDisplay() == DisplayLine)
        return 1;
    int line = 0;
    Element *next = getHead();
    while (next) {
        line += next->getLineNumber();
        next = next->getNext();
    }
    return line;
}

int Element::getWidth(EventContext &context) {
    if (getDisplay() == DisplayBlock || getDisplay() == DisplayLine) {
        if (context.outer) {
            return context.outer->width() - getLogicOffset().x;
        } else {
            return context.doc->getWidth(context) - getLogicOffset().x;
        }
    }
    return Root::getWidth(context);
}

void Element::onPreMouseMove(EventContext &context, int x, int y) {
    for_context(event, context) {
        if (cur_context(event, contain, x, y)) {
            context_on(event, PreMouseMove, x, y);
            return;
        }
    }
    if (!context.doc->getContext()->m_enterRect.round().contains(x, y)) {
        context.doc->getContext()->m_enterRect = context.rect();
        auto *old = context.doc->getContext()->m_enterElement;
        context.doc->getContext()->m_enterElement = this;
        if (old) {
            old->onMouseLeave(x, y);
        }
        onMouseEnter(context, x, y);
    }
    context.current()->onMouseMove(context, x, y); // 触发最内层的函数
}

Root *Root::getContain(EventContext &context, int x, int y) {
    if (!context.canEnter()) {
        return this;
    }
    EventContext event = context.enter();
    while (event.has()) {
        if (event.current()->contain(event, x, y)) {
            return event.current()->getContain(event, x, y);
        }
        event.next();
    }
    return nullptr;
}

void Root::onRedraw(EventContext &context) {
    EventContext ctx = context.enter();
    while (ctx.has()) {
        if (ctx.current()->getDisplay() != DisplayNone) {
            ctx.current()->onRedraw(ctx);
        }
        ctx.next();
    }
}

void Root::onRemove(EventContext &context) {
    if ((Root *) context.doc->getContext()->m_enterElement == this) {
        context.doc->getContext()->m_enterElement->onMouseLeave(0, 0);
        context.doc->getContext()->m_enterElement = nullptr;
    }
}
