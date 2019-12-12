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
#define context_on(ctx, method, ...) ((ctx).current()->on##method(ctx, ##__VA_ARGS__))
// Root 无 父元素
class Root {
public:
    virtual ~Root() = default;
    ///////////////////////////////////////////////////////////////////
    virtual void dump() {}
    virtual void free() { delete this; }
    virtual Tag getTag(EventContext &context) { return Tag(_GT("Element")); }

    // 获取对于父元素的相对偏移
    virtual Offset getLogicOffset() { return {0, 0}; }
    // 获取实际的偏移
    virtual Offset getOffset(EventContext &context) { return {0, 0}; }
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
    virtual Element *getHead() { return nullptr; }
    virtual Element *getTail() { return nullptr; }
    virtual Element *getNext() { return nullptr; }
    virtual Element *getPrev(){ return nullptr; }
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
    virtual void setNext(Element *next) {}
    virtual void setPrev(Element *prev){}
    virtual void setLogicOffset(Offset offset) {}
    virtual Display getDisplay() { return DisplayNone; };
    virtual void onPreMouseMove(EventContext &context, int x, int y);
    virtual void onMouseMove(EventContext &context, int x, int y){}
    virtual void onMouseEnter(EventContext &context, int x, int y) {}
    virtual void onMouseLeave(int x, int y) {}
    // Display为Custom的时候才会调用这个方法
    virtual void onReflow(LayoutArgs()) {}
    DEFINE_EVENT(onFocus);
    DEFINE_EVENT(onBlur);
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
    DEFINE_EVENT(onUndo, Command command);
    DEFINE_EVENT(onEnterReflow, Offset &offset);
    DEFINE_EVENT(onLeaveReflow);
    DEFINE_EVENT(onFinishReflow, int width, int height);
    enum NotifyType {
        None,
        Update,
    };
    DEFINE_EVENT(onNotify, int type, int param, int other);
    virtual Element *copy() { return nullptr; }
    int getLineNumber();
    int getWidth(EventContext &context) override;
};

class LinkedElement : public Element {
protected:
    Element *m_prev = nullptr;
    Element *m_next = nullptr;
public:
    Element *getNext() override { return m_next; }
    Element *getPrev() override { return m_prev; }
    void setNext(Element *next) override { m_next = next; }
    void setPrev(Element *prev) override { m_prev = prev; }
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
            next->free();
            start = next;
        }
        delete this;
    }
    Element *getHead() override { return m_head; }
    Element *getTail() override { return m_tail; }
    void setLogicWidth(EventContext &context, int width) override { m_width = width; }
    void setLogicHeight(EventContext &context, int height) override { m_height = height; }
    int getLogicWidth(EventContext &context) override { return m_width; }
    int getLogicHeight(EventContext &context) override { return m_height; }
    int getHeight(EventContext &context) override { return getLogicHeight(context); }
    int getWidth(EventContext &context) override { return getLogicWidth(context); }
    void onFinishReflow(EventContext &context, int width, int height) override { m_width = width;m_height = height; }
    Display getDisplay() override { return D; }

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
    virtual Element *remove(int index) {
        Element *ele = get(index);
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
    virtual Element *replace(int index, Element *element) {
        Element *before = get(index);
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
};

class Document : public Container<DisplayBlock> {
public:
    Context m_context;
public:
    explicit Document(RenderManager *renderManager) : m_context(renderManager) {}
    ///////////////////////////////////////////////////////////////////
    inline Context *getContext() { return &m_context; };
    void flow() {
        LayoutManager::ReflowAll(this);
    }

    Element *append(Element *element) override {
        int count = element->getLineNumber();
        for (int i = 0; i < count; ++i) {
            m_context.m_textBuffer.appendLine();
        }
        return Container<>::append(element);
    };
    EventContext appendElement(Element *element) {
        EventContext context = EventContextBuilder::build(this);
        context.init(this, false);
        append(element);
        return context;
    }
    LineViewer appendLine(Element *element) {
        if (element->getDisplay() != DisplayLine) {
            return {};
        }
        append(element);
        return m_context.m_textBuffer.getLine(m_context.m_textBuffer.getLineCount() - 1);
    };
    int getLogicWidth(EventContext &context) override {
        return m_context.m_layoutManager.getWidth();
    }
    int getLogicHeight(EventContext &context) override {
        return m_context.m_layoutManager.getHeight();
    }

};

class AbsoluteElement : public Element {
    int m_left{};
    int m_top{};
    int m_width{};
    int m_height{};
public:
    AbsoluteElement(int left, int top, int width, int height) : m_left(left), m_top(top), m_width(width), m_height(height) {}
private:
public:
    Offset getLogicOffset() override { return {m_left, m_top}; }
    int getLogicWidth(EventContext &context) override { return m_width; }
    int getLogicHeight(EventContext &context) override { return m_height; }
};

#endif //TEST_DOCUMENT_H
