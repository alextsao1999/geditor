//
// Created by Alex on 2019/6/25.
//

#ifndef _DOCUMENT_H
#define _DOCUMENT_H

#include <queue>
#include "common.h"
#include "layout.h"
#include "paint_manager.h"
#include "command_queue.h"
#include "text_buffer.h"
#include "caret_manager.h"

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

enum class Display {
    None,
    Inline,
    Block,
    Line,
};

struct Context {
    std::queue<EventContext *> m_animator;
    RenderManager *m_renderManager;
    LayoutManager m_layoutManager;
    CaretManager m_caretManager;
    CommandQueue m_queue;
    TextBuffer m_textBuffer;
    Element *m_mouseEnter = nullptr;
    //////////////////////////
    bool m_selecting = false;
    Offset m_selectStart;
    Offset m_selectEnd;
    void startSelect() {
        m_selecting = true;
        m_selectStart = m_caretManager.getCurrent();
        m_selectEnd = m_selectStart;
    }
    void endSelect() {
        m_selecting = false;
    }
    void selecting() {
        m_selectEnd = m_caretManager.getCurrent();
    }

    inline GRect getSelectRect() {
        Offset start = m_selectStart;
        Offset end = m_selectEnd;

        SkRect rect{};
        rect.set({(SkScalar) start.x, (SkScalar) start.y}, {(SkScalar) end.x, (SkScalar) end.y});
        return rect;
    }
    explicit Context(RenderManager *renderManager) : m_renderManager(renderManager),
                                                     m_caretManager(CaretManager(renderManager)) {}
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
    virtual Element *&at(int index) { return m_buffer[index]; }
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
    Context *getDocContext();
    inline bool empty() { return buffer == nullptr || doc == nullptr; }
    inline bool has() { return buffer!= nullptr && index < buffer->size(); }
    bool prev();
    bool next();
    bool outerNext();
    bool outerPrev();
    int getLineIndex() { if (outer) { return outer->getLineIndex() + line; } else { return line; } }
    // 只改变本层次Line
    void prevLine(int count = 1) { line -= count; }
    void nextLine(int count = 1) { line += count; }
    void reflowBrother();
    void reflow();
    void redraw();
    void focus();
    void combine();
    void push(CommandType type, CommandData data);
    void insert(int idx, Element *ele) { buffer->insert(idx, ele); }
    void notify(int type, int p1, int p2);
    void post();
    GRect rect();
    Offset offset();
    GRect viewportRect();
    Offset viewportOffset();
    Offset relative(int x, int y);
    int width();
    int height();
    void remove(Root *element);
    void init(Root *element, int index = 0);
    Element *get(int idx) { return buffer->at(idx); }
    inline Element *current() { return buffer->at(index); }
    Painter getPainter();
    Canvas getCanvas(SkPaint *paint = nullptr);
    RenderManager *getRenderManager();
    LayoutManager *getLayoutManager();
    CaretManager *getCaretManager();
    LineViewer getLineViewer(int column = 0);
    LineViewer copyLine();
    EventContext() = default;
    explicit EventContext(Document *doc) : doc(doc) {}
    explicit EventContext(Document *doc, ElementIndexPtr buffer, EventContext *out, int idx);
    explicit EventContext(const EventContext *context, EventContext *out);
    EventContext start() { return EventContext(doc, buffer, outer, 0); }
    EventContext enter();
    void leave() {
        if (outer) {
            outer->nextLine(line);
        }
    }
    EventContext *copy() { return new EventContext(this, outer ? outer->copy() : nullptr); }
    void free() {
        if (outer) {
            outer->free();
        }
        delete this;
    }
    void dump() {
        if (outer) {
            outer->dump();
        }
        printf(" { index = %d, line = %d }", index, line);
    }
    bool selecting() {
        return getDocContext()->m_selecting;
    }
};

class EventContextBuilder {
public:
    inline static EventContext build(Document *doc) {
        return EventContext(doc);
    }
};

// Root 无 父元素
class Root {
public:
    virtual ~Root() = default;
    ///////////////////////////////////////////////////////////////////
    virtual bool hasChild() { return false; };
    virtual ElementIndex *children() { return nullptr; }
    inline ElementIterator begin() { if(hasChild()) return children()->begin();return {}; }
    inline ElementIterator end() {  if(hasChild()) return children()->end(); return {}; }
    ///////////////////////////////////////////////////////////////////
    virtual void dump() {}

    // 获取对于父元素的相对偏移
    virtual Offset getLogicOffset() { return {0, 0}; }
    // 获取实际的偏移
    virtual Offset getOffset(EventContext &context) { return {0, 0}; }
    virtual int getLogicWidth(EventContext &context) { return 0; };
    virtual int getLogicHeight(EventContext &context) { return 0; };
    virtual void setLogicWidth(int width) {};
    virtual void setLogicHeight(int height) {};
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
    /////////////////////////////////////////
};

// Element 有 父元素
class Element : public Root {
    friend Document;
public:
    Element() = default;
    inline Offset getRelOffset(EventContext &context, int x, int y) {
        Offset offset(x, y);
        return offset - getOffset(context);
    }
    Offset getOffset(EventContext &context) override;
    virtual void setLogicOffset(Offset offset) {}
    virtual Display getDisplay() { return Display::None; };
    virtual void onPreMouseMove(EventContext &context, int x, int y);
    virtual void onMouseMove(EventContext &context, int x, int y){}
    virtual void onMouseEnter(EventContext &context, int x, int y) {}
    virtual void onMouseLeave(int x, int y) {}
    virtual bool onNextFrame(EventContext &context) { return false; }
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
    enum NotifyType {
        None,
        SizeChange,
        HeightChange,
        WidthChange,
        Update,
    };
    DEFINE_EVENT(onNotify, int type, int p1, int p2);
    virtual Element *copy() { return nullptr; }
    int getLineNumber();
    int getWidth(EventContext &context) override;
    void dump() override {
        std::cout << "Display : ";
        switch (getDisplay()) {
            case Display::None:
                std::cout << "None";
                break;
            case Display::Inline:
                std::cout << "Inline";
                break;
            case Display::Block:
                std::cout << "Block";
                break;
            case Display::Line:
                std::cout << "Line";
                break;
        }
        std::cout << std::endl;

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

};

// 根据Children 自动改变宽高
class Container : public RelativeElement {
public:
    ElementIndex m_index;
    int m_width = 0;
    int m_height = 0;
public:
    Display m_display = Display::Block;
    Container() = default;
    ~Container() override {
        for (auto element : m_index) {
            // FIXME mouseEnter可能和element指向同一个元素
            delete element;
        }
    }
    explicit Container(Display display) : m_display(display) {}
    bool hasChild() override {
        return m_index.size() > 0;
    }
    void setLogicWidth(int width) override { m_width = width; }
    void setLogicHeight(int height) override { m_height = height; }
    int getLogicWidth(EventContext &context) override { return m_width; }
    int getLogicHeight(EventContext &context) override { return m_height; }
    int getHeight(EventContext &context) override { return getLogicHeight(context); }
    int getWidth(EventContext &context) override { return getLogicWidth(context); }
    ElementIndex *children() override { return &m_index; }
    Display getDisplay() override { return m_display; }
    inline void setDisplay(Display display) { m_display = display; }
    virtual void append(Element *element) { m_index.append(element); }
};

class Document : public Container {
public:
    Context m_context;
public:
    explicit Document(RenderManager *renderManager) : m_context(Context(renderManager)) {}
    ///////////////////////////////////////////////////////////////////
    inline Context *getContext() { return &m_context; };
    LineViewer appendLine(Element *element) {
        if (element->getDisplay() != Display::Line) {
            return {};
        }
        m_index.append(element);
        return m_context.m_textBuffer.appendLine();
    };
    void flow() {
        m_context.m_layoutManager.reflowAll(this);
    }
    void append(Element *element) override {
        m_index.append(element);
        int count = element->getLineNumber();
        for (int i = 0; i < count; ++i) {
            m_context.m_textBuffer.appendLine();
        }
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
