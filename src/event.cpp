//
// Created by Alex on 2019/12/12.
//

#include "event.h"
#include "document.h"

#define OutOfBound() (!element)
#define CheckBound(ret) if (OutOfBound()) return ret
LineViewer EventContext::getLineViewer(int column) {
    return doc->getContext()->m_textBuffer.getLine(getLineCounter(), column);
}

void EventContext::init(Element *ele) {
    element = ele->getHead();
    index = 0;
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

bool EventContext::canEnter() {
    return element && (element->getHead() != nullptr);
}

EventContext EventContext::enter(int idx) {
    return EventContext(this, idx);
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
    if (element == nullptr) {
        return false;
    }
    index--;
    element = element->getPrev();
    if (element) {
        prevLine(element->getLineNumber());
    }
    return true;
}
bool EventContext::next() {
    if (element == nullptr) {
        return false;
    }
    index++;
    nextLine(element->getLineNumber());
    element = element->getNext();
    return true;
}
void EventContext::reflow(bool relayout) {
    doc->m_context.m_layoutManager.reflow(*this, relayout);
    doc->getContext()->m_caretManager.update();
}
void EventContext::relayout() {
    doc->m_context.m_layoutManager.relayout(*this);
}
void EventContext::redraw() {
    doc->getContext()->m_renderManager->redraw(this);
}
void EventContext::focus() {
    doc->m_context.m_caretManager.focus(copy());
}
void EventContext::remove(Root *root) {
    root->onRemove(*this);
    root->free();
}
void EventContext::combine() {
/*
    int next = index + 1;
    if (next >= buffer->size()) {
        return;
    }
    auto &text = doc->getContext()->m_textBuffer;
    Element *ele = buffer->at(next);
    auto cur = getLineCounter();
    if (current()->getDisplay() == DisplayLine && ele->getDisplay() == DisplayLine) {
        auto curLineViewer = text.getLine(cur);
        cur.increase(this, 1);
        curLineViewer.append(text.getLine(cur).c_str());
        text.deleteLine(cur);
        buffer->erase(next);
        remove(ele);
    }
*/
}

LineViewer EventContext::copyLine() {
    if (current()->getDisplay() == DisplayLine) {
        auto next = getLineCounter();
        next.increase(this, 1);
        Element *element = current()->copy();
        if (!element) {
            return {};
        }
        //buffer->insert(index + 1, element);
        return doc->getContext()->m_textBuffer.insertLine(next);
    }
    return {};
}

void EventContext::push(CommandType type, CommandData data) {
    doc->getContext()->m_queue.push({copy(), type, data});
}
void EventContext::notify(int type, int param, int other) {
    CheckBound(void(0));
    current()->onNotify(*this, type, param, other);
}
Tag EventContext::tag() {
    if (OutOfBound()) {
        return {};
    }
    return current()->getTag(*this);
}
GRect EventContext::logicRect() {
    CheckBound({});
    //Offset pos = current()->getLogicOffset();
    return GRect::MakeXYWH(0, 0, logicWidth(), logicHeight());
}
GRect EventContext::rect() {
    CheckBound({});
    Offset pos = offset();
    return GRect::MakeXYWH(pos.x, pos.y, width(), height());
}
GRect EventContext::viewportRect() {
    CheckBound({});
    Offset pos = viewportOffset();
    return GRect::MakeXYWH(SkIntToScalar(pos.x), SkIntToScalar(pos.y), SkIntToScalar(width()), SkIntToScalar(height()));
}
Offset EventContext::viewportOffset() {
    CheckBound({});
    return offset() - doc->getContext()->m_renderManager->getViewportOffset();
}
Offset EventContext::offset() {
    CheckBound({});
    return current()->getOffset(*this);
}
Offset EventContext::relative(int x, int y) { return Offset{x, y}  - offset(); }
Display EventContext::display() { CheckBound(DisplayNone);return current()->getDisplay(); }
int EventContext::height() { CheckBound(0);return current()->getHeight(*this); }
int EventContext::width() { CheckBound(0);return current()->getWidth(*this); }
int EventContext::logicHeight() { CheckBound(0);return current()->getLogicHeight(*this); }
int EventContext::logicWidth() { CheckBound(0);return current()->getLogicWidth(*this); }
int EventContext::minHeight() { CheckBound(0);return current()->getMinHeight(*this); }
int EventContext::minWidth() { CheckBound(0);return current()->getMinWidth(*this); }
void EventContext::setLogicHeight(int height) { CheckBound(void(0));current()->setLogicHeight(*this, height); }
void EventContext::setLogicWidth(int width) { CheckBound(void(0));current()->setLogicWidth(*this, width); }

EventContext::EventContext(Document *doc) : doc(doc), element(doc) {}
EventContext::EventContext(EventContext *out, int idx) : doc(out->doc), outer(out) {
    /*更新line*/
    if (idx >= 0) {
        element = outer->current()->getHead();
        for (int j = 0; j < idx; ++j) {
            next();
        }
    } else {
        index = -1;
        element = outer->current()->getTail();
        if (element) {
            counter.increase(this, outer->current()->getLineNumber() - element->getLineNumber());
        }
        for (int j = 0; j < -idx - 1; ++j) {
            prev();
        }
    }
}
EventContext::EventContext(const EventContext *context, EventContext *out) :
        doc(context->doc), element(context->element), index(context->index), counter(context->counter), outer(out) {}

void EventContext::timer(long long interval, int id, int count) {
    std::thread timer([=](EventContext *context, Element *current) {
        int tick = count;
        while (current->onTimer(*context, id)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            if (count && --tick == 0) {
                break;
            }
        }
        context->free();
    }, copy(), current());
    timer.detach();
}

Context *EventContext::getDocContext() {
    return doc->getContext();
}

StyleManager *EventContext::getStyleManager() {
    return &getDocContext()->m_styleManager;
}

bool EventContext::selected() {
    CheckBound(false);
    GRect rect = this->rect();
    GRect selected = getDocContext()->getSelectRect();
    return GRect::Intersects(rect, selected);
}

bool EventContext::visible() {
    CheckBound(false);
    Size size = doc->m_context.m_renderManager->getViewportSize();
    return GRect::Intersects(GRect::MakeWH(size.width, size.height), viewportRect());
}

bool EventContext::selecting() { return getDocContext()->m_selecting; }

Lexer *EventContext::getLexer(int column) {
    getDocContext()->m_lexer.enter(this, column);
    return &getDocContext()->m_lexer;
}

bool EventContext::isHead() { return element && element->getPrev() == nullptr; }
bool EventContext::isTail() { return element && element->getNext() == nullptr; }
