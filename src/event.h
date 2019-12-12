//
// Created by Alex on 2019/12/12.
//

#ifndef GEDITOR_EVENT_H
#define GEDITOR_EVENT_H

#include "common.h"
#include "line_index.h"
#include "command_queue.h"
#include "layout.h"
#include "paint_manager.h"
#include "text_buffer.h"
#include "caret_manager.h"
#include "lexer.h"
#include <vector>
#include <gl/GL.h>

struct Context;
class Root;
class Element;
class Document;
class ElementIndex;
using ElementIterator = std::vector<Element *>::iterator;
using ElementIndexPtr = ElementIndex *;
struct Context {
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
        m_selectStart = m_caretManager.current();
        m_selectEnd = m_selectStart;
    }
    void endSelect() {
        m_selecting = false;
    }
    void selecting() {
        m_selectEnd = m_caretManager.current();
    }

    inline GRect getSelectRect() {
        Offset start = m_selectStart;
        Offset end = m_selectEnd;

        SkRect rect{};
        rect.set({(SkScalar) start.x, (SkScalar) start.y}, {(SkScalar) end.x, (SkScalar) end.y});
        return rect;
    }
    explicit Context(RenderManager *renderManager) : m_renderManager(renderManager),
                                                     m_caretManager(renderManager), m_layoutManager(renderManager){}
};
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
struct Tag {
    GChar str[255]{_GT('\0')};
    Tag() = default;
    Tag(const GChar* string) { gstrcpy(str, string); }
    bool empty() { return gstrlen(str) == 0; }
    bool operator==(const Tag &rvalue) { return gstrcmp(str, rvalue.str); }
    bool operator==(const GChar *rvalue) { return gstrcmp(str, rvalue) == 0; }
    bool contain(const GChar *item) {
        for (int i = 0; str[i] != _GT('\0'); ++i) {
            if (item[0] != str[i])
                continue;
            int j;
            for (j = 0; item[j] != _GT('\0') && str[i + j] != _GT('\0'); ++j) {
                if (item[j] != str[i + j]) {
                    break;
                }
            }
            if (item[j] == _GT('\0')) {
                return true;
            }
        }
        return false;
    }
    Tag &append(const GChar *value) {
        gstrcat(str, value);
        return *this;
    }
    void dump() {
        printf("Tag -> %ws\n", str);
    }
};
struct EventContext {
    Document *doc = nullptr;
    Element *element = nullptr;
    EventContext *outer = nullptr;
    int index = 0;
    LineCounter counter;
    Context *getDocContext();
    inline bool empty() { return element == nullptr || doc == nullptr; }
    inline bool has() { return element; };
    bool prev();
    bool next();
    LineCounter getLineCounter() { if (outer) { return outer->getLineCounter() + counter; } else { return counter; } }
    // 只改变本层次Line
    void prevLine(int count = 1) { counter.decrease(this, count); }
    void nextLine(int count = 1) { counter.increase(this, count); }
    void reflow(bool relayout = false);
    void redraw();
    void relayout();
    void focus();
    void combine();
    void push(CommandType type, CommandData data);
    void notify(int type, int param, int other);
    void timer(long long interval, int id = 0, int count = 0);

    Tag tag();
    GRect rect();
    GRect logicRect();
    Offset offset();
    GRect viewportRect();
    Offset viewportOffset();
    Offset relative(int x, int y);
    Offset absolute(int x, int y) { return {x, y}; } // 将 x, y 转为 绝对offset
    Offset relOffset(Offset abs) {return abs - offset(); }
    Offset absOffset(Offset rel) { return rel + offset(); }
    Display display();
    int width();
    int height();
    int logicWidth();
    int logicHeight();
    int minWidth();
    int minHeight();
    void setLogicWidth(int width);
    void setLogicHeight(int height);
    void remove(Root *ele);
    void init(Element *ele, bool head = true);
    inline CaretPos &pos() { return getCaretManager()->data(); }
    inline Element *current() { return element; }
    Painter getPainter();
    Canvas getCanvas(SkPaint *paint);
    Canvas getCanvas();
    RenderManager *getRenderManager();
    LayoutManager *getLayoutManager();
    CaretManager *getCaretManager();
    StyleManager *getStyleManager();
    inline GStyle &getStyle(int id) { return getStyleManager()->get(id); }
    Lexer *getLexer(int column = 0);
    LineViewer getLineViewer(int column = 0);
    LineViewer copyLine();
    explicit EventContext(Document *doc) : doc(doc) {}
    explicit EventContext(EventContext *out, int idx);
    explicit EventContext(const EventContext *context, EventContext *new_out); // copy context with new outer
    bool canEnter();
    EventContext begin() { return EventContext(outer, 0); }
    EventContext end() { return EventContext(outer, 0); }
    EventContext nearby(int value) { return EventContext(outer, index + value); }
    EventContext enter(int idx = 0);
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
        printf(" { index = %d, line = %d }", index, counter.line);
    }
    std::string path() {
        std::string str;
        if (outer) {
            str.append(outer->path());
        }
        char buf[255] = {"\0"};
        sprintf(buf, "/%d", index);
        str.append(buf);
        return str;
    }

    bool selecting();
    bool selected();
    bool visible();
    bool isHead();
    bool isTail();
    EventContext *findNext(const GChar *tag) {
        EventContext *res = findNextBrother(tag);
        if (res == nullptr && outer) {
            return outer->findNext(tag);
        }
        return res;
    }
    EventContext *findNextBrother(const GChar *tag, bool skipSelf = true) {
        EventContext context = *this;
        if (skipSelf) {
            context.next();
        }
        while (context.has()) {
            if (context.tag().contain(tag)) {
                return context.copy();
            } else {
                if (context.canEnter()) {
                    EventContext *res = context.enter().findNextBrother(tag, false);
                    if (res != nullptr) {
                        return res;
                    }
                }
            }
            context.next();
        }
        return nullptr;
    }
    EventContext *findPrev(const GChar *tag) {
        EventContext *res = findPrevBrother(tag);
        if (res == nullptr && outer) {
            return outer->findPrev(tag);
        }
        return res;
    }
    EventContext *findPrevBrother(const GChar *tag, bool skipSelf = true) {
        EventContext context = *this;
        if (skipSelf) {
            context.prev();
        }
        while (context.has()) {
            if (context.tag().contain(tag)) {
                return context.copy();
            } else {
                if (context.canEnter()) {
                    EventContext *res = context.enter(-1).findPrevBrother(tag, false);
                    if (res != nullptr) {
                        return res;
                    }
                }
            }
            context.prev();
        }
        return nullptr;
    }
};
class EventContextBuilder {
public:
    inline static EventContext build(Document *doc) {
        return EventContext(doc);
    }
};
#endif //GEDITOR_EVENT_H
