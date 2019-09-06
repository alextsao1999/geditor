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

#define CallChildEvent(name) { \
        EventContext event = getContainEventContext(context, x, y); \
        if (!event.empty()) \
            event.current()->name(event, x, y); \
    }

#define DEFINE_EVENT(event, ...) virtual void event(EventContext &context, ##__VA_ARGS__) {}
#define DEFINE_EVENT2(event, ...) virtual void event(EventContext &context, ##__VA_ARGS__) {CallChildEvent(event);}

enum class Display {
    None,
    Inline,
    Block,
    Line,
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

class Element;
class ElementIndex;
using ElementIterator = std::vector<Element *>::iterator;
using ElementIndexPtr = ElementIndex *;
class ElementIndex {
public:
    std::vector<Element *> m_buffer;
public:
    virtual void append(Element *element) { m_buffer.push_back(element); }
    virtual Element *at(int index) { return m_buffer[index]; }
    virtual ElementIterator begin() { return m_buffer.begin(); }
    virtual ElementIterator end() { return m_buffer.end(); }
    virtual void insert(int index, Element *element) { m_buffer.insert(m_buffer.begin() + index, element); }
    virtual void erase(int index) { m_buffer.erase(m_buffer.begin() + index); }
    virtual bool empty() { return m_buffer.empty(); }
    virtual int size() { return m_buffer.size(); }
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
    inline bool empty() { return buffer == nullptr || doc == nullptr; }
    bool has() { return buffer!= nullptr && index < buffer->size(); }
    bool prev();
    bool next();
    int getLine() {
        if (outer) {
            return outer->getLine();
        } else {
            return line;
        }
    }
    void prevLine() {
        if (outer) {
            outer->prevLine();
        } else {
            line--;
        }
    }
    void nextLine() {
        if (outer) {
            outer->nextLine();
        } else {
            line++;
        }
    }
    void reflowBrother();
    void reflow();
    void redraw();
    void focus();
    void combine();
    void push(CommandType type, CommandData data);

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
    LineViewer copyLine();
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
    EventContext enter(Root *element, int index = 0);
    EventContext enter();

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
    virtual bool hasChild() { return false; };
    virtual ElementIndex *children() { return nullptr; }
    inline ElementIterator begin() { if(hasChild()) return children()->begin();return {}; }
    inline ElementIterator end() {  if(hasChild()) return children()->end(); return {}; }
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
    /**
     * 绝对坐标是否包含在元素中
     * @return bool
     */
    virtual bool contain(EventContext &context, int x, int y) {
        Offset offset = getOffset();
        return (offset.x < x) && (offset.x + getWidth(context) > x) && (offset.y < y) &&
               (offset.y + getHeight(context) > y);
    };
    /**
     * 获取绝对坐标所包含的最底层的元素
     * @return Root *
     */
    Root *getContain(EventContext &context, int x, int y);
    /**
     * 获取绝对坐标所在的子元素
     * @return EventContext
     */
    EventContext getContainEventContext(EventContext &context, int x, int y);
    /**
     * 获取相对坐标所在的子元素
     * @return EventContext
     */
    /////////////////////////////////////////
    virtual void redraw(EventContext &context);
    virtual void reflow(EventContext &context);
    /////////////////////////////////////////
};

// Element 有 父元素
class Element : public Root {
    friend Document;
public:
    Element() = default;
    explicit Element(Root *parent) : m_parent(parent) {}
    inline Root *parent() const { return m_parent; }
    inline Offset getRelOffset(int x, int y) {
        Offset offset(x, y);
        return offset - getOffset();
    }
    Offset getOffset() override;
    virtual void setLogicOffset(Offset offset) {}
    virtual Display getDisplay() { return Display::None; };
    DEFINE_EVENT(onFocus);
    DEFINE_EVENT(onBlur);
    DEFINE_EVENT(onKeyDown, int code, int status);
    DEFINE_EVENT(onKeyUp, int code, int status);
    DEFINE_EVENT2(onMouseEnter, int x, int y);
    DEFINE_EVENT2(onMouseMove, int x, int y);
    DEFINE_EVENT2(onMouseLeave, int x, int y);
    DEFINE_EVENT2(onLeftButtonUp, int x, int y);
    DEFINE_EVENT2(onLeftButtonDown, int x, int y);
    DEFINE_EVENT2(onRightButtonUp, int x, int y);
    DEFINE_EVENT2(onRightButtonDown, int x, int y);
    DEFINE_EVENT2(onLeftDoubleClick, int x, int y);
    DEFINE_EVENT2(onRightDoubleClick, int x, int y);
    DEFINE_EVENT(onInputChar, int ch);
    DEFINE_EVENT(onSelect);
    DEFINE_EVENT(onUnselect);
    DEFINE_EVENT(onUndo, Command command);
    virtual Element *copy() { return nullptr; }
    int getLineNumber();
    bool contain(EventContext &context, int x, int y) override {
        Offset offset = getOffset();
        if (getDisplay() == Display::Block || getDisplay() == Display::Line) {
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

    LineViewer appendLine(Element *element) {
        if (element->getDisplay() != Display::Line) {
            return {};
        }
        m_elements.append(element);
        element->m_parent = this;
        return m_context.m_textBuffer.appendLine();
    };

    void append(Element *element) {
        m_elements.append(element);
        element->m_parent = this;
        for (int i = 0; i < element->getLineNumber(); ++i) {
            m_context.m_textBuffer.appendLine();
        }
    };

    Element *getLineElement(int line) {
        return m_elements.at(line);
    }

    void flow() {
        m_context.m_layoutManager.reflowAll(this);
    }

    void reflow(EventContext &context) override {
        // 这里重排 emm...
        context.getLayoutManager()->reflow(context);
    }

    bool hasChild() override {
        return !m_elements.empty();
    }

    ElementIndex *children() override {
        return &m_elements;
    }

    int getLogicWidth(EventContext &context) override {
        // FIXME:
        //  Width is not the size of viewport
        //  it should be the real width
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
