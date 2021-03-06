//
// Created by Alex on 2019/12/12.
//

#include "event.h"
#include "document.h"
#include "doc_manager.h"

#define OutOfBound() (!element)
#define CheckBound(ret) if (OutOfBound()) return ret

LineViewer EventContext::getLineViewer(int offset, bool pushCommand) {
    return document()->buffer()->getLine(line() + offset, pushCommand ? this : nullptr);
}
Painter EventContext::getPainter() { return doc->render()->getPainter(this); }
Canvas EventContext::getCanvas() { return doc->render()->getCanvas(this); }
bool EventContext::canEnter() { return element && element->onCanEnter(*this); }
bool EventContext::hasChild() { return element && element->getHead() != nullptr; }
EventContext EventContext::enter(int idx) { return EventContext(this, idx); }
RenderManager *EventContext::getRenderManager() { return doc->render(); }
LayoutManager *EventContext::getLayoutManager() { return doc->layouter(); }
CaretManager *EventContext::getCaretManager() { return &doc->context()->m_caretManager; }

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
    element = element->onPrev(*this);
    if (element) {
        prevLine(element->getLineNumber());
    }
    return true;
}
bool EventContext::next() {
    if (element == nullptr) {
        return false;
    }
    nextLine(element->getLineNumber());
    element = element->onNext(*this);
    return true;
}
void EventContext::reflow(bool relayout) {
    doc->m_context.m_layoutManager.reflow(*this, relayout, element->getLogicOffset());
    doc->context()->m_caretManager.update(); // reflow之后更新光标位置
}
void EventContext::relayout() {
    element->onRelayout(*this, document()->layouter());
}
void EventContext::redraw() { doc->render()->redraw(this); }
void EventContext::focus(bool isCopy, bool force, EventContext *sender) {
    if (!element) {
        return;
    }
    doc->m_context.m_caretManager.focus(sender, isCopy ? copy() : this, force);
}
void EventContext::push(CommandType type, CommandData data) {
    doc->onContentChange(*this, type, data);
    doc->context()->m_queue.push({copy(), pos(), type, data});
}
void EventContext::notify(int type, NotifyParam param, NotifyValue other) {
    CheckBound(void());
    current()->onNotify(*this, type, param, other);
}
Tag EventContext::tag() {
    CheckBound({});
    return current()->getTag(*this);
}

GRect EventContext::bound(GScalar dx, GScalar dy) {
    GRect grect = GRect::MakeWH(width(), height());
    grect.inset(dx, dy);
    return grect;
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
    return offset() - caretOffset() - doc->render()->getViewportOffset();
}
Offset EventContext::viewportLogicOffset() {
    CheckBound({});
    return element->getLogicOffset() - doc->render()->getViewportOffset();
}
Offset EventContext::offset() {
    CheckBound({});
    return current()->getOffset(*this);
}
Offset EventContext::relative(int x, int y) { return Offset{x, y}  - offset(); }
Display EventContext::display() { CheckBound(DisplayNone);return current()->getDisplay(*this); }
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
    outer->current()->onEnter(*out, *this, idx);
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

Context *EventContext::getDocContext() { return doc->context(); }

StyleManager *EventContext::getStyleManager() { return doc->style(); }

SelectionState EventContext::getSelectionState() {
    CheckBound(SelectionNone);
    auto &&dcontext = getDocContext();
    Offset start = dcontext->getSelectStart();
    Offset end = dcontext->getSelectEnd();

    if (start == end) {
        return SelectionNone;
    }

    GRect a = this->rect();
    Offset caret = caretOffset();
    a.offset(-caret.x, -caret.y);

    GRect b;
    b.set({(float) start.x, (float) start.y}, {(float) end.x, (float) end.y});

    SkIRect rect = a.round();
    SelectionState state = SelectionNone;

    if (a.fTop <= b.fBottom && b.fTop <= a.fBottom) {
        state = SelectionRow;
    }

    if (GRect::Intersects(a, b)) {
        state = SelectionInside;
    }

    if (b.fBottom >= a.fTop && b.fTop <= a.fBottom && a.fLeft <= b.fLeft && a.fRight >= b.fLeft) {
        state = SelectionInside;
    }

    if (b.fRight >= a.fLeft && b.fLeft <= a.fRight && a.fTop <= b.fTop && a.fBottom >= b.fTop) {
        state = SelectionInside;
    }

    if (rect.contains(start.x, start.y)) {
        state = SelectionStart;
    }

    if (rect.contains(end.x, end.y)) {
        state = (state == SelectionStart) ? SelectionSelf : SelectionEnd;
    }

    return state;
}

bool EventContext::isSelected() {
    CheckBound(false);
    GRect a = this->rect();
    Offset caret = caretOffset();
    a.offset(-caret.x, -caret.y);
    GRect b = getDocContext()->getSelectRect();
    if (b.height() == 0) {
        return b.fRight >= a.fLeft && b.fLeft <= a.fRight && a.fTop <= b.fTop && a.fBottom >= b.fTop;
    }
    if (b.width() == 0) {
        return b.fBottom >= a.fTop && b.fTop <= a.fBottom && a.fLeft <= b.fLeft && a.fRight >= b.fLeft;
    }
    return GRect::Intersects(a, b);
}

bool EventContext::isSelectedSelf() {
    return isSelectedStart() && isSelectedEnd();
}

bool EventContext::isSelectedRow() {
    CheckBound(false);
    GRect a = this->rect();
    Offset caret = caretOffset();
    a.offset(-caret.x, -caret.y);
    GRect b = getDocContext()->getSelectRect();
    return a.fTop <= b.fBottom && b.fTop <= a.fBottom && (b.height() != 0);
}

bool EventContext::visible() {
    CheckBound(false);
    auto rect = doc->render()->getViewportRect();
    return GRect::Intersects(rect, viewportRect()) && display() != DisplayNone;
}

bool EventContext::isSelecting() { return getDocContext()->m_selecting; }

GLexer *EventContext::getLexer() {
    getDocContext()->m_lexer.enter(this);
    return &getDocContext()->m_lexer;
}

bool EventContext::isHead() { return element && element->isHead(*this); }
bool EventContext::isTail() { return element && element->isTail(*this); }

void EventContext::replace(Element *new_element, bool pushCommand) {
    if (element) {
        if (pushCommand) {
            EventContext *caret = document()->caret()->getEventContext();
            push(CommandType::ReplaceElement, CommandData(caret->copy(), new_element));
        }
        element = element->onReplace(*this, new_element);
        //before->free();
    }
}
void EventContext::remove(bool pushCommand) {
    if (element) {
        if (pushCommand) {
            push(CommandType::DeleteElement, CommandData(element));
        }
        element->link(*this, element->getNext(), element->getPrev());
        deleteLine(0, element->getLineNumber());
        prevLine(element->getLineNumber());
    }
}
void EventContext::insert(Element *ele, bool pushCommand) {
    if (pushCommand) {
        push(CommandType::AddElement, CommandData(ele));
    }
    Element *next = element->getNext();
    ele->setPrev(element);
    ele->setNext(next);
    element->setNext(ele);
    if (next) {
        next->setPrev(ele);
    } else {
        if (outer) {
            outer->current()->setTail(ele);
        }
    }
    insertLine(element->getLineNumber(), ele->getLineNumber());
}
void EventContext::seperate(bool pushCommand) {
    if (pushCommand) {
        push(CommandType::SeparateElement, CommandData(element));
    }
    element->link(*this, element->getHead(), element->getTail());
    element = element->getHead();
}

bool EventContext::isSelectedStart() {
    return rect().round().contains(doc->m_context.getSelectStart().x, doc->m_context.getSelectStart().y);
}

bool EventContext::isSelectedEnd() {
    return rect().round().contains(doc->m_context.getSelectEnd().x, doc->m_context.getSelectEnd().y);
}

int EventContext::selectedCount() {
    return element->getSelectCount(*this);
}

Offset EventContext::caretOffset() {
    EventContext *pOuter = outer;
    Offset offset;
    while (pOuter != nullptr) {
        offset += pOuter->element->getCaretOffset(*pOuter);
        pOuter = pOuter->outer;
    }
    return offset;
}

bool EventContext::isMouseIn() {
    return rect().round().contains(doc->m_mouse.x, doc->m_mouse.y);
}

bool EventContext::isFocusIn() {
    return compare(document()->caret()->getEventContext());
}

void EventContext::gutter() {
    document()->margin()->drawGutter(this);
}

bool EventContext::contains(int line) {
    return line >= counter.line && line - counter.line < element->getLineNumber();
}

void EventContext::insertLine(int offset, int count) {
    document()->buffer()->insertLines(line() + offset, count);
    document()->onLineChange(*this, count);
}

void EventContext::deleteLine(int offset, int count) {
    document()->buffer()->deleteLines(line() + offset, count);
    document()->onLineChange(*this, -count);
}

void EventContext::breakLine(int offset, int idx, bool pushCommand) {
    if (pushCommand) {
        push(CommandType::Break, CommandData(index));
    }
    auto line = getLineViewer(offset, false);
    auto next = getLineViewer(offset + 1, false);
    next.append(line.c_str() + idx, line.length() - idx);
    line.remove(idx, line.length() - idx);
}

void EventContext::combineLine(int offset, bool pushCommand) {
    auto line = getLineViewer(offset, false);
    if (pushCommand) {
        push(CommandType::Combine, CommandData(line.length()));
    }
    auto next = getLineViewer(offset + 1, false);
    line.append(next.c_str());
    next.clear();
}
