//
// Created by Alex on 2019/6/25.
//

#ifndef _DOCUMENT_H
#define _DOCUMENT_H

#include <vector>
#include "common.h"
#include "layout.h"
#include "paint_manager.h"
#include "command_queue.h"
#include "text_buffer.h"
#include "caret_manager.h"

#define DEFINE_EVENT(event, ...) virtual void event(EventContext &context, ##__VA_ARGS__) {}
#define DECLARE_ACTION() friend LayoutManager; \
    void reflow(EventContext context) override { \
        context.doc->getContext()->m_layoutManager->reflow(context); \
    }

enum class Display {
    None,
    Inline,
    Block,
};

struct Context {
    PaintManager *m_paintManager;
    LayoutManager m_layoutManager;
    CaretManager m_caretManager;
    CommandQueue m_queue;
    TextBuffer m_textBuffer;
    explicit Context(PaintManager *paintManager) : m_paintManager(paintManager),
                                                     m_caretManager(CaretManager(paintManager)) {}
};

struct Element;
using ElementIterator = std::vector<Element *>::iterator;
using ElementIndexPtr = std::vector<Element *> *;
class ElementIndex {
public:
    std::vector<Element *> m_buffer;
public:
    inline void append(Element *element) { m_buffer.push_back(element); }
    inline Element *get(int index) { return m_buffer[index]; }
    inline ElementIterator begin() { return m_buffer.begin(); }
    inline ElementIterator end() { return m_buffer.end(); }
    inline bool empty() { return m_buffer.empty(); }
    inline int getCount() { return m_buffer.size(); }
    inline ElementIndexPtr getPointer() { return &m_buffer; }
};

struct EventContext {
    Document *doc = nullptr;
    ElementIndexPtr buffer = nullptr;
    EventContext *outer = nullptr;
    int index = 0;
    int line = 0;
    EventContext *copy() {
        return new EventContext(this, outer ? outer->copy() : nullptr);
    }
    void free() {
        if (!outer)
            return;
        outer->free();
        delete outer;
        outer = nullptr;
    }
    void jump(int idx) {
        if (idx >= buffer->size())
            return;
        index = idx;
    }
    bool has() { return index < buffer->size(); }
    void next();
    void nextLine() {
        if (outer) {
            outer->nextLine();
        } else {
            line++;
        }
    }
    /**
     * 设置当前上下文对象
     * @param obj
     * @param index
     */
    void set(Root *obj, int index);
    inline Element *current() {
        return buffer->at((unsigned int) index);
    }
    Painter getPainter();
    PaintManager *getPaintManager();
    LayoutManager *getLayoutManager();
    CaretManager *getCaretManager();
    LineViewer getLineViewer();
    EventContext() = default;
    explicit EventContext(Document *doc) : doc(doc) {}
    explicit EventContext(Document *doc, ElementIndexPtr buffer, EventContext *out, int idx) : doc(doc),
                                                                                                buffer(buffer),
                                                                                                outer(out), index(idx) {}
    explicit EventContext(const EventContext *context, EventContext *out) : doc(context->doc),
                                                                            buffer(context->buffer),
                                                                            index(context->index), line(context->line),
                                                                            outer(out) {}
    /**
     * 进入Children
     * @param element
     * @param index
     * @return
     */
    EventContext enter(Element *element, int index = 0);
};


class EventContextBuilder {
public:
    inline static EventContext build(Document *doc) {
        return EventContext(doc);
    }
};

// Root 无 父元素
struct Root {
    ///////////////////////////////////////////////////////////////////
    virtual int getChildrenNum() { return 0; };
    virtual ElementIndex *children() { return nullptr; }
    inline ElementIterator begin() { if(getChildrenNum()) return children()->begin();return {}; }
    inline ElementIterator end() {  if(getChildrenNum()) return children()->end(); return {}; }
    ///////////////////////////////////////////////////////////////////
    virtual void dump() {}
    // 获取实际的偏移
    virtual Offset getOffset() { return {0, 0}; }
    // 获取对于父元素的相对偏移
    virtual Offset getLogicOffset() { return {0, 0}; }
    // 获取实际宽度
    virtual int getWidth(EventContext &context) { return getLogicWidth(context); };
    // 获取实际高度
    virtual int getHeight(EventContext &context) { return getLogicHeight(context); };
    virtual int getLogicWidth(EventContext &context) { return 0; };
    virtual int getLogicHeight(EventContext &context) { return 0; };
    virtual bool contain(EventContext &context, int x, int y) {
        Offset offset = getOffset();
        return (offset.x < x) && (offset.x + getWidth(context) > x) && (offset.y < y) &&
               (offset.y + getHeight(context) > y);
    };
    Root *getContain(EventContext &context, int x, int y);
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
    DEFINE_EVENT(rightDoubleClick, int x, int y);
    /////////////////////////////////////////
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
    inline Root *parent() const { return m_parent; }
    Offset getOffset() override;
    virtual void setLogicOffset(Offset offset) {}
    virtual Display getDisplay() { return Display::None; };

    bool contain(EventContext &context, int x, int y) override {
        Offset offset = getOffset();
        if (getDisplay() == Display::Block) {
            return (offset.x < x) && (m_parent->getOffset().x + m_parent->getWidth(context) > x) && (offset.y < y) &&
                   (offset.y + getHeight(context) > y);
        }
        return (offset.x < x) && (offset.x + getWidth(context) > x) && (offset.y < y) &&
               (offset.y + getHeight(context) > y);
    }

    int getWidth(EventContext &context) override {
        if (getDisplay() == Display::Block) {
            return parent()->getWidth(context) - (getOffset().x - parent()->getOffset().x);
        }
        return Root::getWidth(context);
    }

protected:
    Root *m_parent{};
};

class Document : public Root {
public:
    Context m_context;
    ElementIndex m_elements;

public:
    explicit Document(PaintManager *paintManager) : m_context(Context(paintManager)) {}
    ~Document() {
        for (auto element : m_elements) {
            delete element;
        }
    }
    ///////////////////////////////////////////////////////////////////
    inline Context *getContext() { return &m_context; };

    LineViewer append(Element *element) {
        m_elements.append(element);
        element->m_parent = this;
        return m_context.m_textBuffer.appendLine();
    };

    Element *getLineElement(int line) {
        return m_elements.get(line);
    }

    void flow(int index = 0) {
        if (m_elements.empty())
            return;
        EventContext context = EventContextBuilder::build(this);
        context.set(this, index);
        m_elements.get(index)->reflow(context);
    }

    void reflow(EventContext &context) override {
        // 这里重排 emm...
        context.getLayoutManager()->reflow(context);
    }

    int getChildrenNum() override {
        return m_elements.getCount();
    }

    ElementIndex *children() override {
        return &m_elements;
    }

    int getLogicWidth(EventContext &context) override {
        return m_context.m_paintManager->getViewportSize().width;
    }

    int getLogicHeight(EventContext &context) override {
        return m_context.m_paintManager->getViewportSize().height;
    }

};

class RelativeElement : public Element {
    friend LayoutManager;
    friend Document;
protected:
    Offset m_offset;
public:
    using Element::Element;
    Offset getLogicOffset() override { return m_offset; }
    void setLogicOffset(Offset offset) override { m_offset = offset; }

    void reflow(EventContext &context) override {
        context.getLayoutManager()->reflow(context);
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
