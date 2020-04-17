//
// Created by Alex on 2019/6/25.
//

#include "document.h"
#include "doc_manager.h"
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
    int line = 0;
    Element *next = getHead();
    while (next) {
        line += next->getLineNumber();
        next = next->getNext();
    }
    return line;
}

int Element::getWidth(EventContext &context) {
    if (context.display() == DisplayBlock) {
        if (context.outer) {
            return context.outer->width() - getLogicOffset().x;
        }
    }
    return Root::getWidth(context);
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

void Root::onDraw(EventContext &context, Drawable canvas) {
    for_context(ctx, context) {
        if (ctx.visible()) {
            Offset offset = ctx.current()->getLogicOffset();
            if (context.isDocument()) {
                offset -= context.getRenderManager()->getViewportOffset();
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

}

Document::Document(RenderManager *render, DocumentManager *mgr) :
m_manager(mgr), m_context(render), m_root(this) {}

GString Document::getSelectionString() {
    std::wstringbuf buf;
    std::wostream stream(&buf);
    onSelectionToString(m_root, SelectionNone, stream);
    return buf.str();
}

MarginDocument::MarginDocument(DocumentManager *mgr) :
        Document(mgr->m_render, mgr), m_margin(this) {}

