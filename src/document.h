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
#define DEFINE_EVENT(event, ...) virtual void event(EventContext context, ##__VA_ARGS__) {}
//#define DECLARE_ACTION()  \
// friend LayoutManager; \
// void reflow(Context *context) override { context->m_layoutManager->reflow(this); }
#define DECLARE_ACTION() friend LayoutManager; void reflow(EventContext context) override { context.doc->getContext()->m_layoutManager->reflow(context, this); }

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

using ElementIterator = std::vector<Element *>::iterator;
class ElementIndex {
private:
    std::vector<Element *> m_buffer;
public:
    void append(Element *element) {
        m_buffer.push_back(element);
    }

    inline ElementIterator begin() {
        return m_buffer.begin();
    }
    inline ElementIterator end() {
        return m_buffer.end();
    }

};

class Document : public Root {
public:
    Context m_context{};
    RelativeElement *m_header = nullptr;
    LayoutManager m_layoutManager{this};
    ElementIndex m_elements;

public:
    Document();
    static void FreeAll(RelativeElement *ele);
    ~Document() { FreeAll(m_header); }
    DECLARE_ACTION();
    ElementIterator children() override;
    ///////////////////////////////////////////////////////////////////
    inline Context *getContext();
    void append(RelativeElement *element);
    void flow();
};

struct EventContext {
    Document *doc;
    std::vector<Element *> *m_buffer;

    int line = 0;

    bool has() { return false; }
    void next() {}

    inline Element *current() {
        return m_buffer->at((unsigned int) line);
    }
    LineViewer getLineViewer() {
        return doc->getContext()->m_textBuffer.getLine(line);
    }
};

/*
class ElementIterator {
private:
    ElementIndex *buffer = nullptr;
public:
    ElementIterator() = default;
    inline ElementIterator &operator++() { next();return *this; }
    inline bool operator!=(const ElementIterator &item) { return (!(item.end() && end())) || (pointer() != item.pointer()); }
    inline Element &operator*() const { return *(pointer()); }
    inline Element *operator->() const { return pointer(); }
    inline bool has() { return !end(); }
    virtual Element *pointer() const { return nullptr; }
    virtual bool end() const { return true; }
    virtual void next() {};
};
*/
// Root 无 父元素
struct Root {
    ///////////////////////////////////////////////////////////////////
    virtual ElementIterator children() { return {}; }
    ///////////////////////////////////////////////////////////////////
    virtual void dump() {}
    virtual Offset getOffset() { return {0, 0}; }
    virtual Offset getLogicOffset() { return {0, 0}; }
    virtual int getWidth() { return getLogicWidth(); };
    virtual int getHeight() { return getLogicHeight(); };
    virtual int getLogicWidth() { return 0; };
    virtual int getLogicHeight() { return 0; };
    /////////////////////////////////////////
    DEFINE_EVENT(redraw);
    DEFINE_EVENT(reflow);
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
    Offset getOffset() override;
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


#endif //TEST_DOCUMENT_H
