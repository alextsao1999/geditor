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
    if (getDisplay() == Display::Line)
        return 1;
    if (!hasChild())
        return 0;
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

int Element::getWidth(EventContext &context) {
    if (getDisplay() == Display::Block || getDisplay() == Display::Line) {
        if (context.outer) {
            return context.outer->width() - (context.offset().x - context.outer->offset().x);
        } else {
            return context.doc->getWidth(context) - (context.offset().x - context.doc->getOffset(context).x);
        }
    }
    return Root::getWidth(context);
}

void Element::onPreMouseMove(EventContext &context, int x, int y) {
    for (EventContext event = context.enter(); event.has(); event.next()) {
        if (event.current()->contain(event, x, y)) {
            event.current()->onPreMouseMove(event, x, y);
            // 触发有子元素的元素 一般没有做处理
            event.current()->onMouseMove(event, x, y);
            return;
        }
    }
    if (context.doc->getContext()->m_mouseEnter) {
        if (context.doc->getContext()->m_mouseEnter != this) {
            context.doc->getContext()->m_mouseEnter->onMouseLeave(x, y);
            onMouseEnter(context, x, y);
            context.doc->getContext()->m_mouseEnter = this;
        }
    } else{
        context.doc->getContext()->m_mouseEnter = this;
        onMouseEnter(context, x, y);
    }
    context.current()->onMouseMove(context, x, y); // 触发最顶层的函数

}

LineViewer EventContext::getLineViewer() {
    return doc->getContext()->m_textBuffer.getLine(getLineIndex());
}

void EventContext::set(Root *obj, int idx = 0) {
    if (obj->hasChild()) {
        buffer = obj->children();
        index = idx;
    }
}

Painter EventContext::getPainter() {
    return doc->getContext()->m_renderManager->getPainter(this);
}

Canvas EventContext::getCanvas() {
    return doc->getContext()->m_renderManager->getCanvas(this);
}

EventContext EventContext::enter() {
    return EventContext(doc, current()->children(), this, 0);
}

RenderManager *EventContext::getRenderManager() {
    return doc->getContext()->m_renderManager;
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
    nextLine(current()->getLineNumber());
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
    Element *ele = buffer->at(next);
    int cur = getLineIndex();
    if (current()->getDisplay() == Display::Line && ele->getDisplay() == Display::Line) {
        text.getLine(cur).append(text.getLine(cur + 1).str());
        buffer->erase(next);
        doc->getContext()->m_textBuffer.deleteLine(cur + 1);
        //delete ele;
    }
}

void EventContext::redraw() {
    // onRedraw 需要更新宽度!!!
    doc->getContext()->m_renderManager->refresh();
}

LineViewer EventContext::copyLine() {
    if (current()->getDisplay() == Display::Line) {
        int next = getLineIndex() + 1;
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

void EventContext::notify(int type, int p1, int p2) {
    current()->onNotify(*this, type, p1, p2);
}

Offset EventContext::offset() {
    return current()->getOffset(*this);
}

Offset EventContext::relative(int x, int y) {
    return Offset(x, y) - offset();
}

int EventContext::height() {
    return current()->getHeight(*this);
}

int EventContext::width() {
    return current()->getWidth(*this);
}

EventContext::EventContext(Document *doc, ElementIndexPtr buffer, EventContext *out, int idx) :
doc(doc), buffer(buffer), outer(out), index(idx) {}

EventContext::EventContext(const EventContext *context, EventContext *out) :
        doc(context->doc), buffer(context->buffer), index(context->index), line(context->line), outer(out) {}


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
     EventContext event = context.enter();
     while (event.has()) {
         if (event.current()->contain(event, x, y)) {
             return event;
         }
         event.next();
     }

     return {};
}
void Root::onRedraw(EventContext &context) {
    EventContext ctx = context.enter();
    while (ctx.has()) {
        ctx.current()->onRedraw(ctx);
        ctx.next();
    }
}
