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
struct Context;
struct EventContext;
class Root;
class Element;
class Document;
struct Context {
    RenderManager *m_renderManager;
    LayoutManager m_layoutManager;
    StyleManager m_styleManager;
    CaretManager m_caretManager;
    CommandQueue m_queue;
    Lexer m_lexer;
    TextBuffer m_textBuffer;
    // 应该是记录EventContext 这里有些问题
    Element *m_enterElement = nullptr;
    GRect m_enterRect{};
    //////////////////////////
    bool m_selecting = false;
    Offset m_selectStart;
    CaretPos m_selectStartPos;
    Offset m_selectEnd;
    CaretPos m_selectEndPos;
    void startSelect() {
        m_selecting = true;
        m_selectStart = m_caretManager.current();
        m_selectEnd = m_selectStart;
        m_selectStartPos = m_caretManager.data();
    }
    void endSelect() {
        m_selecting = false;
    }
    void selecting() {
        m_selectEnd = m_caretManager.current();
        m_selectEndPos = m_caretManager.data();
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
    Tag &append(const int value) {
        gsprintf(str + gstrlen(str), _GT("%d"), value);
        return *this;
    }
    void dump() {
        printf("Tag [%ws]\n", str);
    }
};
struct ReverseContext {
    EventContext *context = nullptr;
    ReverseContext *inner = nullptr;
    ReverseContext(EventContext *context, ReverseContext *inner) : context(context), inner(inner) {}
    void free() {
        if (inner) {
            inner->free();
        } else {
            //FIXME context->free();
        }
        delete inner;
    }
};
struct EventContext {
    Document *doc = nullptr;
    Element *element = nullptr;
    EventContext *outer = nullptr;
    int index = 0;
    LineCounter counter;
    Context *getDocContext();
    int count();
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
    void focus(bool isCopy = true, bool force = false);
    void push(CommandType type, CommandData data);
    void notify(int type, int param, int other);
    void timer(long long interval, int id = 0, int count = 0);
    void replace(Element *new_element, bool pushCommand = true);
    void remove(Root *ele);

    Tag tag();
    GRect rect();
    GRect bound(GScalar dx = 0, GScalar dy = 0);
    GRect logicRect();
    Offset offset();
    GRect viewportRect();
    Offset viewportOffset();
    Offset viewportLogicOffset();
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
    inline CaretPos &pos() { return getCaretManager()->data(); }
    void setPos(CaretPos pos) {
        getCaretManager()->data() = pos;
        getCaretManager()->update();
    }
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
    LineViewer getLineViewer(int offset = 0, int column = 0);
    void insertLine(int offset = 0, int count = 1) {
        for (int i = 0; i < count; ++i) {
            getDocContext()->m_textBuffer.insertLine(getLineCounter(), offset);
        }
    }
    void deleteLine(int offset = 0, int count = 1) {
        for (int i = 0; i < count; ++i) {
            getDocContext()->m_textBuffer.deleteLine(getLineCounter(), offset);
        }
    }
    EventContext() = default;
    explicit EventContext(Document *doc);
    explicit EventContext(EventContext *out, int idx);
    explicit EventContext(const EventContext *context, EventContext *new_out); // copy context with new outer
    bool canEnter();
    EventContext begin() { return EventContext(outer, 0); }
    EventContext end() { return EventContext(outer, -1); }
    EventContext nearby(int value) {
        //return EventContext(outer, index + value);
        EventContext context = *this;
        int count = value >= 0 ? value : -value;
        for (int k = 0; k < count; ++k) {
            if (value > 0) {
                context.next();
            } else {
                context.prev();
            }
        }
        return context;
    }
    EventContext enter(int idx = 0);
    EventContext parent() { return outer ? *outer : EventContext(); }
    EventContext *copy() {
        if (outer == nullptr) {
            // this is root
            return this;
        }
        return new EventContext(this, outer->copy());
    }
    bool isDocument() { return outer == nullptr && doc != nullptr; }
    void free() {
        if (outer) {
            outer->free();
            delete this;
        }
    }
    void dump() {
        if (outer) {
            outer->dump();
        }
        printf(" { index = %d, line = %d, cur = %p }", index, counter.line, element);
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

    static EventContext *Parse(Document *doc, const char *string) {
        if (string[0] == '/') {
            int index = atoi(string + 1);
            do {
                string++;
                if (string[0] == '/') {
                    break;
                }
            } while (string[0] != '\0');
            return new EventContext(doc);

        }
        return nullptr;
    }

    bool selecting();
    bool isSelected();
    bool isSelectedStart();
    bool isSelectedEnd();
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

    template <typename Type>
    inline Type *cast() { return (Type *) element; }
    bool compare(EventContext *rvalue) {
        if (rvalue == nullptr) {
            return false;
        }
        if (outer && rvalue->outer) {
            return element == rvalue->element && index == rvalue->index && outer->compare(rvalue->outer);
        } else {
            return element == rvalue->element && index == rvalue->index;
        }
    }
    EventContext *include(EventContext *rvalue) {
        if (rvalue) {
            EventContext *find = include(rvalue->element);
            return (find && find->compare(rvalue)) ? find : nullptr;
        }
        return nullptr;
    }
    EventContext *include(Element *rvalue) {
        if (rvalue == nullptr || empty())
            return nullptr;
        EventContext *start = this;
        while (start != nullptr) {
            if (start->element == rvalue) {
                return start;
            }
            start = start->outer;
        }
        return nullptr;
    }
    ReverseContext *reverse() {
        EventContext *start = copy();
        ReverseContext *reverse = nullptr;
        while (start != nullptr) {
            reverse = new ReverseContext(start, reverse);
            start = start->outer;
        }
        return reverse;
    }

    Offset caretOffset();
};
class EventContextBuilder {
public:
    inline static EventContext build(Document *doc) {
        return EventContext(doc);
    }
};

#endif //GEDITOR_EVENT_H
