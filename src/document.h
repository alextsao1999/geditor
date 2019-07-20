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
#define DEFINE_EVENT(event, ...) virtual void event(Document *doc, EventContext context, ##__VA_ARGS__) {}
#define DECLARE_ACTION() friend LayoutManager; void reflow(Context *context) override { context->m_layoutManager->reflow(this); }
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

struct EventContext {
    LineViewer *viewer;

};

// Root 无 父元素
struct Root {
    ///////////////////////////////////////////////////////////////////
    class Iterator {
    public:
        Iterator() = default;
        inline Iterator &operator++() { next();return *this; }
        inline bool operator!=(const Iterator &item) { return (!(item.end() && end())) || (pointer() != item.pointer()); }
        inline Element &operator*() const { return *(pointer()); }
        inline Element *operator->() const { return pointer(); }
        inline bool has() { return !end(); }
        virtual Element *pointer() const { return nullptr; }
        virtual bool end() const { return true; }
        virtual void next() {};
    };
    using IteratorPtr = std::unique_ptr<Iterator>;
    virtual IteratorPtr children() { return nullptr; }
    ///////////////////////////////////////////////////////////////////
    virtual void dump() {}
    virtual Offset getOffset() { return {0, 0}; }
    virtual Offset getLogicOffset() { return {0, 0}; }
    virtual int getWidth() { return getLogicWidth(); };
    virtual int getHeight() { return getLogicHeight(); };
    virtual int getLogicWidth() { return 0; };
    virtual int getLogicHeight() { return 0; };
    /////////////////////////////////////////
    virtual void reflow(Context *context) { context->m_layoutManager->reflow(this); }
    DEFINE_EVENT(enterReflow);
    DEFINE_EVENT(redraw);
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
// Element 有 父元素
class Element : public Root {
    friend Document;
public:
    Element() = default;
    explicit Element(Root *parent) : m_parent(parent) {}
    DECLARE_ACTION();
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
    virtual void setLogicOffset(Offset offset) {}
    virtual Display getDisplay() { return Display::None; };
protected:
    Root *m_parent{};
};

class RelativeElement : public Element {
    friend Document;
protected:
    Offset m_offset;
    RelativeElement *m_prev = nullptr;
    RelativeElement *m_next = nullptr;
public:
    using Element::Element;
    void setLogicOffset(Offset offset) override { m_offset = offset; }
    DECLARE_ACTION();
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
    int getLogicWidth() override { return m_width; }
    int getLogicHeight() override { return m_height; }
};

class Document : public Root {
public:
    Context m_context{};
    RelativeElement *m_header = nullptr;
    LayoutManager m_layoutManager{this};
    std::vector<RelativeElement *> m_rel;
    std::vector<AbsoluteElement *> m_abs;
public:
    Document() { m_context.m_layoutManager = &m_layoutManager; }
    static void FreeAll(RelativeElement *ele) {
        if (ele == nullptr) {
            return;
        }
        FreeAll(ele->m_next);
        delete ele;
    }
    ~Document() { FreeAll(m_header); }
    DECLARE_ACTION();
    ///////////////////////////////////////////////////////////////////
    class DocumentIterator : public Iterator {
    private:
        RelativeElement *m_current = nullptr;
        std::vector<AbsoluteElement *> &m_abs;
    public:
        DocumentIterator(RelativeElement *m_current, std::vector<AbsoluteElement *> &m_abs) : m_current(m_current), m_abs(m_abs) {}
        Element *pointer() const override { return m_current; }
        bool end() const override { return m_current == nullptr; }
        void next() override { m_current = m_current->m_next; }
    };
    IteratorPtr children() override {
        return std::make_unique<DocumentIterator>(m_header, m_abs);
    }
    ///////////////////////////////////////////////////////////////////
    inline Context *getContext() { return &m_context; }
    void append(RelativeElement *element) {
        element->m_parent = this;
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
    void flow() { m_header->reflow(getContext()); }

};

#endif //TEST_DOCUMENT_H
