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
#include "lexer.h"
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

struct Context {
    std::queue<EventContext *> m_animator;
    RenderManager *m_renderManager;
    LayoutManager m_layoutManager;
    StyleManager m_styleManager;
    CaretManager m_caretManager;
    CommandQueue m_queue;
    Lexer m_lexer;
    TextBuffer m_textBuffer;
    Element *m_enterElement = nullptr;
    GRect m_enterRect{};
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
    void reflow(bool relayout = false);
    void redraw();
    void relayout();
    void focus();
    void combine();
    void push(CommandType type, CommandData data);
    void insert(int idx, Element *ele) { buffer->insert(idx, ele); }
    void notify(int type, int param, int other);
    void post();

    GRect rect();
    GRect logicRect();
    Offset offset();
    GRect viewportRect();
    Offset viewportOffset();
    Offset relative(int x, int y);
    Display display();
    int width();
    int height();
    int logicWidth();
    int logicHeight();
    int minWidth();
    int minHeight();
    void setLogicWidth(int width);
    void setLogicHeight(int height);
    void remove(Root *element);
    void init(Root *element, int index = 0);
    Element *get(int idx) { return buffer->at(idx); }
    inline Element *current() { return buffer->at(index); }
    Painter getPainter();
    Canvas getCanvas(SkPaint *paint = nullptr);
    RenderManager *getRenderManager();
    LayoutManager *getLayoutManager();
    CaretManager *getCaretManager();
    StyleManager *getStyleManager();
    inline GStyle &getStyle(int id) { return getStyleManager()->get(id); }
    Lexer *getLexer(int column = 0) {
        getDocContext()->m_lexer.enter(this, column);
        return &getDocContext()->m_lexer;
    }
    LineViewer getLineViewer(int column = 0);
    LineViewer copyLine();
    EventContext() = default;
    explicit EventContext(Document *doc) : doc(doc) {}
    explicit EventContext(Document *doc, ElementIndexPtr buffer, EventContext *out, int idx);
    explicit EventContext(const EventContext *context, EventContext *out);
    EventContext begin() { return EventContext(doc, buffer, outer, 0); }
    EventContext end() { return EventContext(doc, buffer, outer, buffer->size() - 1); }
    EventContext enter(int idx = 0);
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
    std::string path() {
        std::string str;
        if (outer) {
            str.append(outer->path());
        }
        char buf[255] = {"\0"};
        sprintf(buf, "/%d\0", index);
        str.append(buf);
        return str;
    }

    bool selecting() { return getDocContext()->m_selecting; }
    bool selected();
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
    /////////////////////////////////////////
};

// Element 有 父元素
class Element : public Root {
    friend Document;
public:
    Element() = default;
    inline Offset getRelOffset(EventContext &context, int x, int y) { return Offset(x, y) - getOffset(context); }
    Offset getOffset(EventContext &context) override;
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

class RelativeElement : public Element {
    friend LayoutManager;
    friend Document;
protected:
    Offset m_offset;
public:
    using Element::Element;
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
    ElementIndex m_index;
    int m_width = 0;
    int m_height = 0;
public:
    Container() = default;
    ~Container() override {
        for (auto element : m_index) {
            // FIXME mouseEnter可能和element指向同一个元素
            delete element;
        }
    }
    bool hasChild() override { return m_index.size() > 0; }
    void setLogicWidth(EventContext &context, int width) override { m_width = width; }
    void setLogicHeight(EventContext &context, int height) override { m_height = height; }
    int getLogicWidth(EventContext &context) override { return m_width; }
    int getLogicHeight(EventContext &context) override { return m_height; }
    int getHeight(EventContext &context) override { return getLogicHeight(context); }
    int getWidth(EventContext &context) override { return getLogicWidth(context); }
    void onFinishReflow(EventContext &context, int width, int height) override { m_width = width;m_height = height; }
    ElementIndex *children() override { return &m_index; }
    Display getDisplay() override { return D; }
    virtual void append(Element *element) {
        m_index.append(element);
    }
};

class Document : public Container<DisplayBlock> {
public:
    Context m_context;
public:
    explicit Document(RenderManager *renderManager) : m_context(Context(renderManager)) {}
    ///////////////////////////////////////////////////////////////////
    inline Context *getContext() { return &m_context; };
    void flow() {
        LayoutManager::ReflowAll(this);
    }
    void append(Element *element) override {
        m_index.append(element);
        int count = element->getLineNumber();
        for (int i = 0; i < count; ++i) {
            m_context.m_textBuffer.appendLine();
        }
    };
    EventContext appendElement(Element *element) {
        EventContext context = EventContextBuilder::build(this);
        context.init(this, m_index.size());
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
