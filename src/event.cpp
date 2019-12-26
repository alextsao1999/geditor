//
// Created by Alex on 2019/12/12.
//

#include "event.h"
#include "document.h"

#define OutOfBound() (!element)
#define CheckBound(ret) if (OutOfBound()) return ret

LineViewer EventContext::getLineViewer(int offset, int column) {
    return doc->getContext()->m_textBuffer.getLine(getLineCounter(), offset, column);
}
void EventContext::init(Element *ele) {
    element = ele->enterHead();
    index = 0;
}
Painter EventContext::getPainter() { return doc->getContext()->m_renderManager->getPainter(this); }
Canvas EventContext::getCanvas(SkPaint *paint) {
    return doc->getContext()->m_renderManager->getCanvas(this, paint);
}
Canvas EventContext::getCanvas() { return doc->getContext()->m_renderManager->getCanvas(this); }
bool EventContext::canEnter() { return element && (element->getHead() != nullptr); }
EventContext EventContext::enter(int idx) { return EventContext(this, idx); }
RenderManager *EventContext::getRenderManager() { return doc->getContext()->m_renderManager; }
LayoutManager *EventContext::getLayoutManager() { return &doc->getContext()->m_layoutManager; }
CaretManager *EventContext::getCaretManager() { return &doc->getContext()->m_caretManager; }

int EventContext::count() {
    if (element)
        return element->getChildCount();
    return 0;
}

bool EventContext::prev() {
    // prev失败时 并不会改变索引 因此不需要next
    if (element == nullptr) {
        return false;
    }
    element = element->getPrevWithContext(*this);
    if (element) {
        prevLine(element->getLineNumber());
        index--;
    }
    return true;
}
bool EventContext::next() {
    if (element == nullptr) {
        return false;
    }
    index++;
    nextLine(element->getLineNumber());
    element = element->getNextWithContext(*this);
    return true;
}
void EventContext::reflow(bool relayout) {
    doc->m_context.m_layoutManager.reflow(*this, relayout);
    doc->getContext()->m_caretManager.update();
}
void EventContext::remove(Root *root) {
    root->onRemove(*this);
    root->free();
}
void EventContext::relayout() { doc->m_context.m_layoutManager.relayout(*this);reflow(); }
void EventContext::redraw() { doc->getContext()->m_renderManager->redraw(this); }
void EventContext::focus(bool isCopy) { doc->m_context.m_caretManager.focus(isCopy ? copy() : this); }
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
        element = outer->current()->enterHead();
        for (int j = 0; j < idx; ++j) {
            next();
        }
    } else {
        index = -1;
        element = outer->current()->enterTail();
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

Context *EventContext::getDocContext() { return doc->getContext(); }

StyleManager *EventContext::getStyleManager() { return &getDocContext()->m_styleManager; }

bool EventContext::isSelected() {
    CheckBound(false);
    GRect a = this->rect();
    GRect b = getDocContext()->getSelectRect();
    return a.fTop <= b.fBottom && b.fTop <= a.fBottom;
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

bool EventContext::isHead() { return element && element->isHead(*this); }
bool EventContext::isTail() { return element && element->isTail(*this); }

Element *EventContext::replace(Element *new_element) {
    if (element) {
        Element *before = element;
        element = element->onReplace(*this, new_element);
        return before;
    } else {
        return nullptr;
    }
}

bool EventContext::isSelectedStart() {
    return rect().round().contains(doc->m_context.m_selectStart.x, doc->m_context.m_selectStart.y);
}

bool EventContext::isSelectedEnd() {
    return rect().round().contains(doc->m_context.m_selectEnd.x, doc->m_context.m_selectEnd.y);
}

