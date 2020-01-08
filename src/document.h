//
// Created by Alex on 2019/6/25.
//

#ifndef _DOCUMENT_H
#define _DOCUMENT_H

#include <queue>
#include <mutex>
#include "common.h"
#include "event.h"

#define CallChildEvent(name) \
    EventContext event = context.enter(); \
    while (event.has()) { \
        if (event.current()->contain(event, x, y)) { \
            event.current()->name(event, x, y); \
            break; \
        } \
        event.next(); \
    }

#define DEFINE_EVENT(event, ...) virtual void event(EventContext &context, ##__VA_ARGS__) {}
#define DEFINE_EVENT2(event) virtual void event(EventContext &context, int x, int y) {CallChildEvent(event);}
#define CallEvent(context, event, ...) ((context).current()->event(context, ##__VA_ARGS__))
#define for_context(new_ctx, ctx) for (auto new_ctx = (ctx).enter(); new_ctx.has(); new_ctx.next())
#define cur_context(ctx, method, ...) ((ctx).current()->method(ctx, ##__VA_ARGS__))
#define context_on(ctx, method, ...) {auto &&__ctx = (ctx);if (!__ctx.empty())__ctx.current()->on##method(__ctx, ##__VA_ARGS__);}
// Root 无 父元素
class Root {
public:
    virtual ~Root() = default;

    ///////////////////////////////////////////////////////////////////
    virtual void dump() {}
    virtual void free() {}
    virtual Tag getTag(EventContext &context) { return {_GT("Element")}; }

    // 获取对于父元素的相对偏移
    virtual Offset getLogicOffset() { return {0, 0}; }
    // 获取实际的偏移
    virtual Offset getOffset(EventContext &context) { return {0, 0}; }
    virtual Offset getCaretOffset(EventContext &context) { return {0, 0}; }
    virtual int getLogicWidth(EventContext &context) { return 0; };
    virtual int getLogicHeight(EventContext &context) { return 0; };
    virtual int getMinWidth(EventContext &context) { return getLogicWidth(context); };
    virtual int getMinHeight(EventContext &context) { return getLogicHeight(context); };

    // reflow 结束时 会调用此方法 传递最大的行高度或最大的块宽度
    virtual void setLogicWidth(EventContext &context, int width) {};
    virtual void setLogicHeight(EventContext &context, int height) {};
    // 获取实际宽度
    virtual int getWidth(EventContext &context) { return getLogicWidth(context); };
    // 获取实际高度
    virtual int getHeight(EventContext &context) { return getLogicHeight(context); };
    /**
     * 绝对坐标是否包含在元素中
     * @return bool
     */
    virtual bool contain(EventContext &context, int x, int y) {
        return context.rect().round().contains(x, y);
    };
    /**
     * 获取绝对坐标所包含的最底层的元素
     * @return Root *
     */
    Root *getContain(EventContext &context, int x, int y);
    /**
     * 获取相对坐标所在的子元素
     * @return EventContext
     */
    /////////////////////////////////////////
    typedef SkCanvas *Drawable;
    virtual void onDraw(EventContext &context, Drawable canvas);
    virtual void onRedraw(EventContext &context);
    virtual void onRemove(EventContext &context);
    virtual bool onTimer(EventContext &context, int id) { return false; }
    /////////////////////////////////////////
};

// Element 有 父元素
class Element : public Root {
    friend Document;
public:
    Element() = default;
    Offset getOffset(EventContext &context) override;
    virtual int getChildCount() {
        int count = 0;
        Element *start = getHead();
        while (start != nullptr) {
            count++;
            start = start->getNext();
        }
        return count;
    }
    virtual Element *getHead() { return nullptr; }
    virtual Element *getTail() { return nullptr; }
    virtual void setHead(Element *ele) {}
    virtual void setTail(Element *ele) {}
    virtual Element *getNext() { return nullptr; }
    virtual Element *getPrev(){ return nullptr; }
    virtual bool isHead(EventContext &context) { return getPrev() == nullptr; }
    virtual bool isTail(EventContext &context) { return getNext() == nullptr; }
    virtual Element *getNextWithContext(EventContext &context) { return getNext(); }
    virtual Element *getPrevWithContext(EventContext &context){ return getPrev(); }
    virtual void onEnter(EventContext &context, EventContext &enter, int idx) {
        if (idx >= 0) {
            enter.index = 0;
            enter.element = getHead();
            for (int j = 0; j < idx; ++j) {
                enter.next();
            }
        } else {
            enter.index = -1;
            enter.element = getTail();
            if (enter.element) {
                enter.counter.increase(&enter, context.current()->getLineNumber() - enter.element->getLineNumber());
            }
            for (int j = 0; j < -idx - 1; ++j) {
                enter.prev();
            }
        }
    }
    Element *getNextCount(int count) {
        Element *next = this;
        while (count-- && next) {
            next = next->getNext();
        }
        return next;
    }
    Element *getPrevCount(int count) {
        Element *prev = this;
        while (count-- && prev) {
            prev = prev->getPrev();
        }
        return prev;
    }
    virtual void setNext(Element *next){}
    virtual void setPrev(Element *prev){}
    virtual void setLogicOffset(Offset offset) {}
    virtual Display getDisplay() { return DisplayNone; };
    virtual void onPreMouseMove(EventContext &context, int x, int y);
    virtual void onMouseMove(EventContext &context, int x, int y){}
    virtual void onMouseEnter(EventContext &context, int x, int y) {}
    virtual void onMouseLeave(int x, int y) {}
    // Display为Custom的时候才会调用这个方法
    virtual void onReflow(LayoutArgs()) {}
    virtual void onRelayout(EventContext &context, LayoutManager *sender) {
        sender->reflow(context.enter(), true);
    }
    DEFINE_EVENT(onFocus);
    DEFINE_EVENT(onBlur, EventContext *focus);
    DEFINE_EVENT(onKeyDown, int code, int status);
    DEFINE_EVENT(onKeyUp, int code, int status);
    DEFINE_EVENT2(onLeftButtonUp);
    DEFINE_EVENT2(onLeftButtonDown);
    DEFINE_EVENT2(onRightButtonUp);
    DEFINE_EVENT2(onRightButtonDown);
    DEFINE_EVENT2(onLeftDoubleClick);
    DEFINE_EVENT2(onRightDoubleClick);
    DEFINE_EVENT(onInputChar, int ch);
    DEFINE_EVENT(onSelect);
    DEFINE_EVENT(onUnselect);
    //DEFINE_EVENT(onUndo, Command command);
    DEFINE_EVENT(onEnterReflow, Offset &offset);
    DEFINE_EVENT(onLeaveReflow, Offset &offset);
    DEFINE_EVENT(onFinishReflow, int width, int height);
    virtual void onUndo(Command command) {}
    virtual void onUndoDiscard(Command command) {}
    virtual Element *onReplace(EventContext &context, Element *element) { return nullptr; }
    enum NotifyType {
        None,
        Update,
    };
    DEFINE_EVENT(onNotify, int type, int param, int other);
    virtual int getLineNumber();
    int getWidth(EventContext &context) override;
};
class LinkedElement : public Element {
protected:
    Element *m_prev = nullptr;
    Element *m_next = nullptr;
public:
    void free() override { delete this; }
    Element *getNext() override { return m_next; }
    Element *getPrev() override { return m_prev; }
    void setNext(Element *next) override { m_next = next; }
    void setPrev(Element *prev) override { m_prev = prev; }
    Element *onReplace(EventContext &context, Element *new_element) override {
        if (new_element == nullptr) {
            return this;
        }
        /* //删除元素(暂时不需要)
                if (new_element == nullptr) {
                    if (m_prev) m_prev->setNext(m_next); else context.outer->current()->setHead(m_next);
                    if (m_next) m_next->setPrev(m_prev); else context.outer->current()->setTail(m_prev);
                    return m_next;
                }
        */
        onRemove(context);
        int old_line = getLineNumber();
        int new_line = new_element->getLineNumber();
        int delta = new_line - old_line;
        if (delta > 0) {
            context.insertLine(old_line, delta);
        } else {
            context.deleteLine(new_line, -delta);
        }
        new_element->setPrev(m_prev);
        new_element->setNext(m_next);
        if (m_prev) {
            m_prev->setNext(new_element);
        } else {
            if (context.outer) {
                context.outer->current()->setHead(new_element);
            }
        }
        if (m_next) {
            m_next->setPrev(new_element);
        } else {
            if (context.outer) {
                context.outer->current()->setTail(new_element);
            }
        }
        return new_element;
    }
};
class RelativeElement : public LinkedElement {
    friend LayoutManager;
    friend Document;
protected:
    Offset m_offset;
public:
    using LinkedElement::LinkedElement;
    Offset getLogicOffset() override { return m_offset; }
    void setLogicOffset(Offset offset) override { m_offset = offset; }
    void dump() override {
        const char *string[] = {"None", "Inline", "Block", "Line", "Table", "Row"};
        printf("{ display: %s,  offset-x: %d, offset-y: %d }\n", string[getDisplay()], m_offset.x, m_offset.y);
    }
};
class AbsoluteElement : public LinkedElement {
    int m_left{};
    int m_top{};
    int m_right{};
    int m_bottom{};
public:
    AbsoluteElement(int left, int top, int right, int bottom) : m_left(left), m_top(top), m_right(right), m_bottom(bottom) {}
private:
public:
    Display getDisplay() override { return DisplayAbsolute; }
    Offset getLogicOffset() override { return {m_left, m_top}; }
    Offset getOffset(EventContext &context) override {
        Offset offset = getLogicOffset();
        if (context.outer) {
            if (offset.x < 0) {
                offset.x += context.outer->width();
            }
            if (offset.y < 0) {
                offset.y += context.outer->height();
            }
            offset += context.outer->offset();
        }
        return offset;
    }
    int getLogicWidth(EventContext &context) override {
        int width = m_right - m_left;
        if (width < 0 && context.outer) {
            width += context.outer->width() + 1;
        }
        return width;
    }
    int getLogicHeight(EventContext &context) override {
        int height = m_bottom - m_top;
        if (height < 0) {
            height += context.outer->height() + 1;
        }
        return height;
    }
};

// 根据Children 自动改变宽高
template <Display D = DisplayBlock>
class Container : public RelativeElement {
public:
    Element *m_head = nullptr;
    Element *m_tail = nullptr;
    int m_width = 0;
    int m_height = 0;
public:
    Container() = default;
    void free() override {
        Element *start = m_head;
        while (start != nullptr) {
            Element *next = start->getNext();
            start->free();
            start = next;
        }
        delete this;
    }
    Element *getHead() override { return m_head; }
    Element *getTail() override { return m_tail; }
    void setHead(Element *ele) override { m_head = ele; }
    void setTail(Element *ele) override { m_tail = ele; }
    void setLogicWidth(EventContext &context, int width) override { m_width = width; }
    void setLogicHeight(EventContext &context, int height) override { m_height = height; }
    int getLogicWidth(EventContext &context) override { return m_width; }
    int getLogicHeight(EventContext &context) override { return m_height; }
    int getHeight(EventContext &context) override { return getLogicHeight(context); }
    int getWidth(EventContext &context) override { return getLogicWidth(context); }
    void onFinishReflow(EventContext &context, int width, int height) override { m_width = width;m_height = height; }
    Display getDisplay() override { return D; }
    virtual bool contain(Element *element) {
        Element *header = element->getHead();
        while (header != nullptr) {
            if (header == element) {
                return true;
            }
            header = header->getNext();
        }
        return false;
    }
    virtual Element *append(Element *element) {
        if (m_tail == nullptr) {
            m_tail = m_head = element;
        } else {
            m_tail->setNext(element);
            element->setPrev(m_tail);
            m_tail = element;
        }
        return element;
    }
    virtual Element *get(int index) {
        return index >= 0 ? m_head->getNextCount(index) : m_tail->getPrevCount(-index - 1);
    }
    virtual Element *remove(Element *ele) {
        if (ele == nullptr) {
            return nullptr;
        }
        if (ele == m_head) {
            m_head = m_head->getNext();
            m_head->setPrev(nullptr);
        } else {
            ele->getPrev()->setNext(ele->getNext());
        }
        if (ele == m_tail) {
            m_tail = m_tail->getPrev();
            m_tail->setNext(nullptr);
        } else {
            ele->getNext()->setPrev(ele->getPrev());
        }
        return ele;
    }
    virtual Element *replace(Element *before, Element *element) {
        if (before == nullptr) {
            return nullptr;
        }
        element->setNext(before->getNext());
        element->setPrev(before->getPrev());
        if (before == m_head) {
            m_head = element;
        } else {
            before->getPrev()->setNext(element);
        }
        if (before == m_tail) {
            m_tail = element;
        } else {
            before->getNext()->setPrev(element);
        }
        return before;
    }
    virtual Element *remove(int index) { return remove(get(index)); }
    virtual Element *replace(int index, Element *element) { return replace(get(index), element); }
};
template <Display D = DisplayBlock>
class ScrolledContainer : public Container<D> {
public:
    Offset m_contentOffset;
    bool m_click = false;
    Offset m_clickOffset;
    int m_contentWidth = 0;
    int m_contentHeight = 0;
    ScrolledContainer(int width, int height) {
        m_width = width;
        m_height = height;
    }
    void onFinishReflow(EventContext &context, int width, int height) override {
        m_contentWidth = width;
        m_contentHeight = height;
    }
    void onPreMouseMove(EventContext &context, int x, int y) override {
        if (m_click) {
            m_contentOffset = context.relative(x, y) - m_clickOffset;
            m_contentOffset.x = 0;
            context.getCaretManager()->update();
            context.redraw();
            return;
        }
        Element::onPreMouseMove(context, x + m_contentOffset.x, y + m_contentOffset.y);
    }
    void onLeftButtonUp(EventContext &context, int x, int y) override {
        m_click = false;
        Element::onLeftButtonUp(context, x + m_contentOffset.x, y + m_contentOffset.y);
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        m_clickOffset = context.relative(x, y);
        if (m_clickOffset.x > m_width - 10) {
            m_contentOffset.y = m_clickOffset.y;
            m_click = true;
            printf("click %d", m_clickOffset.y);
            return;
        }
        Element::onLeftButtonDown(context, x + m_contentOffset.x, y + m_contentOffset.y);
    }
    void onRightButtonUp(EventContext &context, int x, int y) override {
        Element::onRightButtonUp(context, x + m_contentOffset.x, y + m_contentOffset.y);
    }
    void onRightButtonDown(EventContext &context, int x, int y) override {
        Element::onRightButtonDown(context, x + m_contentOffset.x, y + m_contentOffset.y);
    }
    void onLeftDoubleClick(EventContext &context, int x, int y) override {
        Element::onLeftDoubleClick(context, x + m_contentOffset.x, y + m_contentOffset.y);
    }
    void onRightDoubleClick(EventContext &context, int x, int y) override {
        Element::onRightDoubleClick(context, x + m_contentOffset.x, y + m_contentOffset.y);
    }
    void onDraw(EventContext &context, SkCanvas *canvas) override {
        canvas->translate(0, -m_contentOffset.y);
        Root::onDraw(context, canvas);
        canvas->translate(0, m_contentOffset.y);

        SkPaint border;

        border.setAlpha(150);
        border.setStyle(SkPaint::kFill_Style);
        GRect rect = GRect::MakeXYWH(m_width - 10, m_contentOffset.y + 5, 8, 25);
        canvas->drawRoundRect(rect, 3, 3, border);

        border.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(context.bound(0.5, 0.5), border);

    }
    Offset getCaretOffset(EventContext &context) override {
        return {0, m_contentOffset.y};
    }
};
class Document : public Container<DisplayBlock> {
public:
    Context m_context;
    Offset m_viewportOffset;
    EventContext m_root;
    EventContext m_begin;
public:
    explicit Document(RenderManager *renderManager) : m_context(renderManager), m_root(this) {}
    Tag getTag(EventContext &context) override { return {_GT("Document")}; }
    ///////////////////////////////////////////////////////////////////
    inline Context *getContext() { return &m_context; };
    void flow() { LayoutManager::ReflowAll(this); }
    Offset getLogicOffset() override { return {10, 10}; }
    Element *append(Element *element) override {
        int count = element->getLineNumber();
        for (int i = 0; i < count; ++i) {
            m_context.m_textBuffer.appendLine();
        }
        Container<>::append(element);
        if (m_begin.empty()) {
            m_begin = m_root.enter();
        }
        return element;
    }
    EventContext appendElement(Element *element) {
        append(element);
        return m_root.enter(-1);
    }
    int getLogicWidth(EventContext &context) override {
        return m_context.m_renderManager->getViewportSize().width - 50;
    }
    void onFinishReflow(EventContext &context, int width, int height) override {
        Container::onFinishReflow(context, width, height + 200);
        m_context.m_renderManager->setVertScroll(m_height);
    }
    void setViewportOffset(Offset offset) {
        if (m_viewportOffset.y > offset.y) { // 向上
            while (m_begin.has()) {
                if (!m_begin.visible()) {
                    m_begin.next();
                    break;
                }
                m_begin.prev();
            }
        } else { // 向下
            while (m_begin.has()) {
                if (m_begin.visible()) {
                    break;
                }
                m_begin.next();
            }
        }
        if (m_begin.empty()) {
            m_begin = m_root.enter();
        }
        m_viewportOffset = offset;
    }
    void undo() {
        if (m_context.m_queue.has()) {
            auto command = m_context.m_queue.pop();
            command.context->current()->onUndo(command);
        }

    }
};

#endif //TEST_DOCUMENT_H
