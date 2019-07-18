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
    LayoutManager *m_layoutManager;
    CommandQueue m_queue;
    TextBuffer m_textBuffer;

};

struct Root {
    ///////////////////////////////////////////////////////////////////
    template <typename Item>
    class Iterator {
    public:
        Iterator() = default;
        inline Iterator<Item> &operator++() { next();return *this; }
        inline bool operator!=(const Iterator<Item> &item) { return (!(item.end() && end())) || (pointer() != item.pointer()); }
        inline Element &operator*() const { return *(pointer()); }
        inline Element *operator->() const { return pointer(); }
        inline bool has() { return !end(); }
        virtual Element *pointer() const { return nullptr; }
        virtual bool end() const { return true; }
        virtual void next() {};
    };
    ///////////////////////////////////////////////////////////////////
    virtual void dump() {}

    virtual Offset getOffset() { return {0, 0}; }
    virtual Offset getLogicOffset() { return {0, 0}; }
    virtual Rect getRect() { return getLogicRect(); }
    virtual Rect getLogicRect() { return {0, 0, 0, 0}; }
    virtual Iterator<Element *> children() { return {}; }
    inline Iterator<Element *> begin() { return children(); }
    inline Iterator<Element *> end() { return {}; }
    bool isViewport(Context *context) { return context->m_paintManager->isViewport(getRect()); }
    /////////////////////////////////////////
    DEFINE_EVENT(redraw);
    DEFINE_EVENT(reflow, RelativeElement *begin);
    /////////////////////////////////////////
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

class Element : public Root {
public:
    Element() = default;
    explicit Element(Root *parent) : m_parent(parent) {}
    inline Root *parent() const { return m_parent; }
    Offset getOffset() override {
        Offset offset = getLogicOffset();
        if (m_parent != nullptr) {
            Offset base = m_parent->getOffset();
            offset.x += base.x;
            offset.y += base.y;
        }
        return offset;
    }
protected:
    Root *m_parent{};
};

class RelativeElement : public Element {
    friend Document;
protected:
    RelativeElement *m_prev = nullptr;
    RelativeElement *m_next = nullptr;
public:
    RelativeElement() = default;
    RelativeElement(Root *parent, RelativeElement *prev) : Element(parent), m_prev(prev) {}
    explicit RelativeElement(RelativeElement *prev) : m_prev(prev) {}
    RelativeElement(RelativeElement *_prev, RelativeElement *_next) : m_prev(_prev), m_next(_next) {}
    virtual Display getDisplay() { return Display::None; };
    virtual int getWidth() { return 0; };
    virtual int getHeight() { return 0; };
};

class FixedElement : public RelativeElement {
public:
    explicit FixedElement(FixedElement *prev) : RelativeElement(prev->parent(), prev), m_top(prev->getRect().bottom) {
        prev->m_next = this;
    }
    explicit FixedElement(Root *parent, FixedElement *prev) : RelativeElement(parent, prev), m_top(prev->getRect().bottom) {
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
    Display getDisplay() override { return Display::Block; }
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
    Offset getLogicOffset() override {
        return {m_left, m_top};
    }
    Rect getLogicRect() override {
        return {m_left, m_top, m_width, m_height};
    }
    Rect getRect() override {
        return {getOffset(), m_width, m_height};
    }

};

class Document : public Root {
private:
    Context m_context{};
    RelativeElement *m_header = nullptr;
    std::vector<RelativeElement *> m_rel;
    std::vector<AbsoluteElement *> m_abs;
public:

    ///////////////////////////////////////////////////////////////////
    class DocumentIterator : public Iterator<Element *> {
    private:
        RelativeElement *m_current = nullptr;
        std::vector<AbsoluteElement *> &m_abs;
        int index = 0;
    public:
        DocumentIterator(RelativeElement *m_current, std::vector<AbsoluteElement *> &m_abs) : m_current(m_current), m_abs(m_abs) {}
        Element *pointer() const override { return m_current; }
        bool end() const override { return m_current == nullptr; }
        void next() override { m_current = m_current->m_next; }
    };
    ///////////////////////////////////////////////////////////////////

    Rect getLogicRect() override {
        Size size = m_context.m_paintManager->getViewportSize();
        return {0, 0, size.width, size.height};
    }

    inline Context *getContext() { return &m_context; }

    void append(RelativeElement *element) {
        RelativeElement *ele = m_header;
        while (ele != nullptr && ele->m_next != nullptr)
            ele = ele->m_next;
        if (ele != nullptr) {
            element->m_prev = ele;
            ele->m_next = element;
        } else {
            m_header = element;
        }
    }

    Iterator<Element *> children() override {
        return DocumentIterator(m_header, m_abs);
    }

};

// 现在没什么用
//////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////

#endif //TEST_DOCUMENT_H
