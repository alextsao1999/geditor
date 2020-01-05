//
// Created by Alex on 2019/6/25.
//

#include "document.h"

Offset Element::getOffset(EventContext &context) {
    Offset offset = getLogicOffset();
    EventContext *outer = context.outer;
    while (outer != nullptr) {
        offset += outer->element->getLogicOffset();
        outer = outer->outer;
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

void Root::onDraw(EventContext &context, SkCanvas *canvas) {
    for_context(ctx, context) {
        if (ctx.visible()) {
            Offset offset = ctx.current()->getLogicOffset();
            if (context.isDocument()) {
                offset -= context.doc->getContext()->m_renderManager->getViewportOffset();
            }
            auto bound = GRect::MakeXYWH(offset.x, offset.y, ctx.width(), ctx.height());
            int count = canvas->saveLayer(&bound, nullptr);
            canvas->translate(offset.x, offset.y);
            ctx.current()->onDraw(ctx, canvas);
            canvas->restoreToCount(count);
        }
    }
}

void Root::onRedraw(EventContext &context) {
    for_context(ctx, context) {
        if (ctx.visible()) {
            ctx.current()->onRedraw(ctx);
        }
    }
}

void Root::onRemove(EventContext &context) {
    if (context.doc->getContext()->m_enterElement == this) {
        context.doc->getContext()->m_enterElement->onMouseLeave(0, 0);
        context.doc->getContext()->m_enterElement = nullptr;
    }

}
