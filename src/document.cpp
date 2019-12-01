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
    if (!hasChild())
        return 0;
    int line = 0;
    for (auto ele : *children()) {
        switch (ele->getDisplay()) {
            case DisplayLine:
                line++;
                break;
            case DisplayBlock:
                line += ele->getLineNumber();
            default:
                break;
        }
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

LineViewer EventContext::getLineViewer(int column) {
    return doc->getContext()->m_textBuffer.getLine(getLineIndex(), column);
}

void EventContext::init(Root *element, int idx) {
    if (element->hasChild()) {
        buffer = element->children();
        index = idx;
    }
}

Painter EventContext::getPainter() {
    return doc->getContext()->m_renderManager->getPainter(this);
}

Canvas EventContext::getCanvas(SkPaint *paint) {
    return doc->getContext()->m_renderManager->getCanvas(this, paint);
}

Canvas EventContext::getCanvas() {
    return doc->getContext()->m_renderManager->getCanvas(this);
}

EventContext EventContext::enter(int idx) {
    return EventContext(doc, current()->children(), this, idx);
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
    // prev失败时 并不会改变索引 因此不需要next
    if (index > 0) {
        index--;
        prevLine(current()->getLineNumber());
        return true;
    }
    return false;
}

bool EventContext::next() {
    // next失败时 要prev一下 因为当前的索引已经改变为下一个index
    nextLine(current()->getLineNumber());
    index++;
    return index < buffer->size();
}

bool EventContext::outerNext() {
    if (outer) {
        outer->next();
        buffer = outer->current()->children();
    }
    return index < buffer->size();
}
bool EventContext::outerPrev() {
    if (outer) {
        outer->prev();
        buffer = outer->current()->children();
    }
    return index < buffer->size();
}

void EventContext::reflow(bool relayout) {
    doc->m_context.m_layoutManager.reflow(*this, relayout);
}
void EventContext::relayout() {
    doc->m_context.m_layoutManager.relayout(*this);
}

void EventContext::redraw() {
    // onRedraw 需要更新宽度!!!
    doc->getContext()->m_renderManager->redraw(this);
}

void EventContext::focus() {
    doc->m_context.m_caretManager.focus(this);
}

void EventContext::remove(Root *element) {
    element->onRemove(*this);
    delete element;
}

void EventContext::combine() {
    int next = index + 1;
    if (next >= buffer->size()) {
        return;
    }
    auto &text = doc->getContext()->m_textBuffer;
    Element *ele = buffer->at(next);
    int cur = getLineIndex();
    if (current()->getDisplay() == DisplayLine && ele->getDisplay() == DisplayLine) {
        text.getLine(cur).append(text.getLine(cur + 1).c_str());
        text.deleteLine(cur + 1);
        buffer->erase(next);
        remove(ele);
    }
}

LineViewer EventContext::copyLine() {
    if (current()->getDisplay() == DisplayLine) {
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

void EventContext::notify(int type, int param, int other) {
    current()->onNotify(*this, type, param, other);
}

GRect EventContext::logicRect() {
    Offset pos = current()->getLogicOffset();
    return GRect::MakeXYWH(pos.x, pos.y, width(), height());
}

GRect EventContext::rect() {
    Offset pos = offset();
    return GRect::MakeXYWH(pos.x, pos.y, width(), height());
}

GRect EventContext::viewportRect() {
    Offset pos = viewportOffset();
    return GRect::MakeXYWH(SkIntToScalar(pos.x), SkIntToScalar(pos.y), SkIntToScalar(width()), SkIntToScalar(height()));
}

Offset EventContext::viewportOffset() {
    return offset() - doc->getContext()->m_renderManager->getViewportOffset();
}

Offset EventContext::offset() {
    return current()->getOffset(*this);
}

Offset EventContext::relative(int x, int y) {
    return Offset{x, y}  - offset();
}
Display EventContext::display() {
    return current()->getDisplay();
}

int EventContext::height() {
    return current()->getHeight(*this);
}

int EventContext::width() {
    return current()->getWidth(*this);
}

int EventContext::logicHeight() {
    return current()->getLogicHeight(*this);
}

int EventContext::logicWidth() {
    return current()->getLogicWidth(*this);
}

int EventContext::minHeight() {
    return current()->getMinHeight(*this);
}

int EventContext::minWidth() {
    return current()->getMinWidth(*this);
}

void EventContext::setLogicHeight(int height) {
    current()->setLogicHeight(*this, height);
}

void EventContext::setLogicWidth(int width) {
    current()->setLogicWidth(*this, width);
}

EventContext::EventContext(Document *doc, ElementIndexPtr buffer, EventContext *out, int idx) :
doc(doc), buffer(buffer), outer(out), index(idx) {}

EventContext::EventContext(const EventContext *context, EventContext *out) :
        doc(context->doc), buffer(context->buffer), index(context->index), line(context->line), outer(out) {}

void EventContext::post() {
    doc->getContext()->m_animator.push(copy());
}

Context *EventContext::getDocContext() {
    return doc->getContext();
}

StyleManager *EventContext::getStyleManager() {
    return &getDocContext()->m_styleManager;
}

bool EventContext::selected() {
    GRect rect = this->rect();
    GRect selected = getDocContext()->getSelectRect();
    return GRect::Intersects(rect, selected);
}

Root *Root::getContain(EventContext &context, int x, int y) {
    if (!hasChild()) {
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
