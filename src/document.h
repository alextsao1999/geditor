//
// Created by Alex on 2019/6/25.
//

#ifndef _DOCUMENT_H
#define _DOCUMENT_H

#include <vector>
#include "common.h"
#include "paint_manager.h"
#include "command_queue.h"
#include "layout.h"
#include "text_buffer.h"
#define DEFINE_EVENT(event, ...) virtual void event(Context *context, LineViewer *viewer, ##__VA_ARGS__) {}
enum class Display {
    None,
    Inline,
    Block,
};

struct Context {
    PaintManager *m_paintManager;
    CommandQueue *m_queue;
    LayoutManager *m_layoutManager;
    TextBuffer *m_textBuffer;

};

struct ElementRoot {
    virtual Rect getRect() { return {0, 0, 0, 0}; }
    virtual Rect getLogicRect() { return {0, 0, 0, 0}; }
    virtual std::vector<Element *> *children() { return nullptr; };
    bool isViewport(Context *context) { return context->m_paintManager->isViewport(getRect()); }
    DEFINE_EVENT(draw);
    DEFINE_EVENT(mouseEnter);
    DEFINE_EVENT(mouseMove);
    DEFINE_EVENT(mouseLeave);
    DEFINE_EVENT(leftClick, int x, int y);
    DEFINE_EVENT(leftDoubleClick, int x, int y);
    DEFINE_EVENT(rightClick, int x, int y);
    DEFINE_EVENT(rightDouble_click, int x, int y);
    DEFINE_EVENT(select);
    DEFINE_EVENT(unselect);
    DEFINE_EVENT(input, const char *string);
    DEFINE_EVENT(undo, Command command);
};

class Element : public ElementRoot {
public:
    Element() = default;
    explicit Element(ElementRoot *parent) : m_parent(parent) {}
    inline ElementRoot *parent() const { return m_parent; }
protected:
    ElementRoot *m_parent{};
};

class RelativeElement : public Element {
protected:
    RelativeElement *m_prev{};
    RelativeElement *m_next{};
public:
    RelativeElement() = default;
    RelativeElement(ElementRoot *parent, RelativeElement *prev) : Element(parent), m_prev(prev) {}
    explicit RelativeElement(RelativeElement *prev) : m_prev(prev) {}
    RelativeElement(RelativeElement *_prev, RelativeElement *_next) : m_prev(_prev), m_next(_next) {}
    virtual Display getDisplay() { return Display::None; };
    virtual int getWidth() { return 0; };
    virtual int getHeight() { return 0; };
};

class InlineRelativeElement : public RelativeElement {
public:
    using RelativeElement::RelativeElement;
    Rect getRect() override {
        Rect rect = getLogicRect();
        if (m_prev != nullptr) {
            Rect prev = m_prev->getRect();
            rect.left += prev.right;
            rect.top += prev.bottom;
        } else {
            Rect base = m_parent->getRect();
            rect.top += base.top;
            rect.left += base.left;
        }
        rect.bottom = rect.top + getHeight();
        rect.right = rect.left + getWidth();
        return rect;
    }
    int getWidth() override { return 0; };
    int getHeight() override { return 0; };
    Display getDisplay() override { return Display::Inline; }
};

class BlockRelativeElement : public RelativeElement {
public:
    using RelativeElement::RelativeElement;
    Rect getRect() override {
        if (m_prev != nullptr) {
            Rect prev = m_prev->getRect();
            if (m_prev->getDisplay() != Display::Block)
                prev.left = prev.right;
            prev.top = prev.bottom;
            prev.right = prev.left + getWidth();
            prev.bottom = prev.top + getHeight();
            return prev;
        }
        Rect rect = getLogicRect();
        if (m_parent != nullptr) {
            Rect base = m_parent->getRect();
            rect.left += base.left;
            rect.top += base.top;
        }
        rect.right = rect.left + getWidth();
        rect.bottom = rect.top + getHeight();
        return rect;
    }
    Display getDisplay() override { return Display::Block; }
};

class FixedElement : public BlockRelativeElement {
public:
    explicit FixedElement(FixedElement *prev) : BlockRelativeElement(prev->parent(), prev), m_top(prev->getRect().bottom) {
        prev->m_next = this;
    }
    explicit FixedElement(ElementRoot *parent, FixedElement *prev) : BlockRelativeElement(parent, prev),
                                                                m_top(prev->getRect().bottom) {
        prev->m_next = this;
    }
    inline void setTop(Context *context, int top) {
        m_top = top;
    }
    int getHeight() override {
        return 0;
    }
    Rect getLogicRect() override {
        if (m_parent != nullptr) {
            return {0, m_top, m_parent->getRect().width(), getHeight()};
        }
        NOT_REACHED();
        return {0, m_top, 0, getHeight()};
    }
private:
    int m_top = 0;
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
    Rect getLogicRect() override {
        return {m_left, m_top, m_width, m_height};
    }
    Rect getRect() override {
        if (m_parent != nullptr) {
            Rect parent = m_parent->getRect();
            parent.left += m_left;
            parent.top += m_top;
            parent.right = parent.left + m_width;
            parent.bottom = parent.top + m_height;
            return parent;
        }
        return {0, 0, 0, 0};
    }
};

class Document : public ElementRoot {
private:
    Context m_context;
    std::vector<Element *> m_buffer;
public:
    Rect getLogicRect() override {
        Size size = m_context.m_paintManager->getViewportSize();
        return {0, 0, size.width, size.height};
    }
    inline Context *getContext() {
        return &m_context;
    }
    void append(FixedElement *element) {
        m_buffer.push_back(element);
    }
    void insert(int index, FixedElement *element) {
        m_buffer.insert(m_buffer.begin() + index, element);
    }
    FixedElement *erase(int index) {
        auto ele = m_buffer.begin() + index;
        m_buffer.erase(ele);
        return (FixedElement *) *ele;
    }
    std::vector<Element *> *children() override {
        return &m_buffer;
    }
};


#endif //TEST_DOCUMENT_H
