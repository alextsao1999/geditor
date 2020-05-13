//
// Created by Alex on 2019/12/12.
//

#ifndef GEDITOR_EVENT_H
#define GEDITOR_EVENT_H

#include <protocol.h>
#include "common.h"
#include "line_index.h"
#include "command_queue.h"
#include "layout.h"
#include "paint_manager.h"
#include "text_buffer.h"
#include "caret_manager.h"
#include "glexer.h"
#include "keymap.h"
typedef void *NotifyParam;
typedef int32_t NotifyValue;
struct Context;
struct EventContext;
class Root;
class Element;
class Document;
enum SelectionState {
    SelectionNone,
    SelectionStart,
    SelectionEnd,
    SelectionSelf,
    SelectionInside,
    SelectionRow,
};
static const char *SelectionString[] = {
        "SelectionNone",
        "SelectionStart",
        "SelectionEnd",
        "SelectionSelf",
        "SelectionInside",
        "SelectionRow",
};
enum PushType : int {
    PushTypeNone,
    PushTypeSelect
};
struct Context {
    KeyMap m_keyMap;
    RenderManager *m_renderManager;
    LayoutManager m_layoutManager;
    StyleManager m_styleManager;
    CaretManager m_caretManager;
    CommandQueue m_queue;
    GLexer m_lexer;
    TextBuffer m_textBuffer;
    //////////////////////////
    bool m_selecting = false;
    CaretPos m_selectBasePos;
    CaretPos m_selectCurrentPos;
    CaretPos m_selectStartPos;
    CaretPos m_selectEndPos;
    void select(CaretPos start, CaretPos end) {
        m_selecting = false;
        m_selectBasePos = start;
        m_selectCurrentPos = end;
        if (m_selectCurrentPos.offset.y > m_selectBasePos.offset.y) {
            m_selectStartPos = m_selectBasePos;
            m_selectEndPos = m_selectCurrentPos;
        } else {
            m_selectStartPos = m_selectCurrentPos;
            m_selectEndPos = m_selectBasePos;
        }
        if (m_selectCurrentPos.offset.y == m_selectBasePos.offset.y) {
            if (m_selectStartPos.index > m_selectEndPos.index) {
                std::swap(m_selectStartPos, m_selectEndPos);
            }
        }
        m_renderManager->refresh();
    }
    void startSelect() {
        m_selecting = true;
        m_selectBasePos = m_caretManager.data();
        m_selectStartPos = m_selectBasePos;
        m_selectEndPos = m_selectBasePos;
    }
    void endSelect() {
        m_selecting = false;
    }
    void clearSelect() {
        m_selectBasePos = m_selectStartPos = m_selectEndPos = {};
        m_selecting = false;
    }
    void updateSelect() {
        m_selectCurrentPos = m_caretManager.data();
        if (m_selectCurrentPos.offset.y > m_selectBasePos.offset.y) {
            m_selectStartPos = m_selectBasePos;
            m_selectEndPos = m_selectCurrentPos;
        } else {
            m_selectStartPos = m_selectCurrentPos;
            m_selectEndPos = m_selectBasePos;
        }
        if (m_selectCurrentPos.offset.y == m_selectBasePos.offset.y) {
            if (m_selectStartPos.index > m_selectEndPos.index) {
                std::swap(m_selectStartPos, m_selectEndPos);
            }
        }
    }
    inline bool hasSelection() { return m_selectBasePos.offset != m_selectCurrentPos.offset; }
    inline Offset getSelectStart() { return m_selectStartPos.offset; }
    inline Offset getSelectEnd() { return m_selectEndPos.offset; }
    inline GRect getSelectRect() {
        Offset start = getSelectStart();
        Offset end = getSelectEnd();

        SkRect rect{};
        rect.set({(SkScalar) start.x, (SkScalar) start.y}, {(SkScalar) end.x, (SkScalar) end.y});
        return rect;
    }
    void pushStart(PushType type = PushType::PushTypeNone) {
        m_queue.push({nullptr, m_selectBasePos, CommandType::PushStart, CommandData(type)});
    }
    void pushEnd() {
        m_queue.push({nullptr, m_selectCurrentPos, CommandType::PushEnd, CommandData(0)});
    }
    explicit Context(RenderManager *renderManager) :
    m_renderManager(renderManager),
    m_caretManager(renderManager),
    m_layoutManager(renderManager){}
};
#define TAG(t) (char *)(t)
typedef char *TagId;
struct Tag {
    char str[256] = {'\0'};
    constexpr Tag() = default;
    Tag(TagId string) { strcpy(str, string); }
    Tag(std::string& string) { strcpy(str, string.c_str()); }
    bool empty() { return strlen(str) == 0; }
    Tag &operator=(const Tag&rhs) {
        strcpy(str, rhs.str);
        return *this;
    }
    Tag &operator=(const char *rhs) {
        strcpy(str, rhs);
        return *this;
    }
    bool operator==(const Tag &rvalue) { return strcmp(str, rvalue.str); }
    bool operator==(TagId rvalue) { return strcmp(str, rvalue) == 0; }
    const char *c_str() { return str; }
    bool contain(const TagId value) {
        int s_len = strlen(value);
        int length = int(strlen(str) - s_len);
        for (int i = 0; i <= length; ++i) {
            if (memcmp(str + i, value, s_len) == 0) {
                return true;
            }
        }
        return false;
    }
    Tag &append(TagId value) {
        GASSERT(strlen(str) < 255, "Length >= 256!");
        strcat(str, value);
        return *this;
    }
    Tag &append(int value) {
        GASSERT(strlen(str) < 255, "Length >= 256!");
        sprintf(str + strlen(str), TAG("%d"), value);
        return *this;
    }
    void remove(TagId value) {
        int s_len = strlen(value);
        int length = int(strlen(str) - s_len);
        for (int i = 0; i <= length; ++i) {
            if (memcmp(str + i, value, s_len) == 0) {
                memcpy(str + i, str + i + s_len, length - i + 1);
            }
        }
    }
    void clear() {
        memset(str, 0, 255);
    }
    void dump() {
        printf("Tag [%s]\n", str);
    }
    template <typename ...Args>
    void format(const char *format, Args...args) {
        sprintf(str, format, std::forward(args)...);
    }

    template <typename ...Args>
    static Tag Format(char *format, Args...args) {
        Tag tag;
        sprintf(tag.str, format, args...);
        return tag;
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
    inline Document *document() { return doc; }
    Context *getDocContext();
    int count();
    inline bool empty() { return element == nullptr || doc == nullptr; }
    inline bool has() { return element; };
    bool prev();
    bool next();
    int line() { return getCounter().line; }
    LineCounter getCounter() { if (outer) { return outer->getCounter() + counter; } else { return counter; } }
    // 只改变本层次Line
    void prevLine(int count = 1) { counter.decrease(this, count); }
    void nextLine(int count = 1) { counter.increase(this, count); }
    void reflow(bool relayout = false);
    void redraw();
    void relayout();
    void focus(bool isCopy = true, bool force = false, EventContext *sender = nullptr);
    void push(CommandType type, CommandData data);
    void notify(int type, NotifyParam param, NotifyValue other);
    void timer(long long interval, int id = 0, int count = 0);
    void replace(Element *new_element, bool pushCommand = true);
    void remove(bool pushCommand = true);
    void insert(Element *ele, bool pushCommand = true);
    void seperate(bool pushCommand = true);
    void update() {
        relayout();
        reflow();
        redraw();
    }
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
    Position position() { return {line(), pos().getIndex()}; }
    Painter getPainter();
    Canvas getCanvas();
    RenderManager *getRenderManager();
    LayoutManager *getLayoutManager();
    CaretManager *getCaretManager();
    StyleManager *getStyleManager();
    inline GStyle &getStyle(int id = StyleDeafaultFont) { return getStyleManager()->get(id); }
    GLexer *getLexer();
    LineViewer getLineViewer(int offset = 0, bool pushCommand = true);
    void insertLine(int offset = 0, int count = 1);
    void deleteLine(int offset = 0, int count = 1);
    void breakLine(int offset, int idx, bool pushCommand = true);
    void combineLine(int offset = 0, bool pushCommand = true);
    EventContext() = default;
    explicit EventContext(Document *doc);
    explicit EventContext(EventContext *out, int idx);
    explicit EventContext(const EventContext *context, EventContext *new_out); // copy context with new outer
    bool canEnter();
    bool hasChild();
    EventContext begin() { return EventContext(outer, 0); }
    EventContext end() { return EventContext(outer, -1); }
    EventContext nearby(int value = 1) {
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
        printf("{%d,tag=[%s]} ", index, tag().str);
    }
    std::string path() {
        std::string str;
        if (outer) {
            str.append(outer->path());
        }
        char buf[255] = {"\0"};
        sprintf(buf, "/%d[%s]", index, tag().str);
        str.append(buf);
        return str;
    }

    bool isSelecting();
    bool isMouseIn();
    bool isFocusIn();
    SelectionState getSelectionState();
    bool isSelected();
    bool isSelectedRow();
    bool isSelectedSelf();
    bool isSelectedStart();
    bool isSelectedEnd();
    int selectedCount();
    bool visible();
    bool isHead();
    bool isTail();
    EventContext *findNext(TagId tag) {
        EventContext *res = findNextBrother(tag);
        if (res == nullptr && outer) {
            return outer->findNext(tag);
        }
        return res;
    }
    EventContext *findNextBrother(TagId tag, bool skipSelf = true) {
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
    EventContext *findPrev(TagId tag) {
        EventContext *res = findPrevBrother(tag);
        if (res == nullptr && outer) {
            return outer->findPrev(tag);
        }
        return res;
    }
    EventContext *findPrevBrother(TagId tag, bool skipSelf = true) {
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
    EventContext *findInnerFirst(TagId tag) {
        for (EventContext inner = enter(); inner.has(); inner.next()) {
            if (inner.tag().contain(tag)) {
                return inner.copy();
            } else {
                if (inner.canEnter()) {
                    EventContext *find = inner.findInnerFirst(tag);
                    if (find) {
                        return find;
                    }
                }
            }
        }
        return nullptr;
    }
    EventContext *findInnerLast(TagId tag) {
        if (this->tag().contain(tag)) {
            return copy();
        }
        for (EventContext inner = enter(-1); inner.has(); inner.prev()) {
            EventContext *find = inner.findInnerLast(tag);
            if (find) {
                return find;
            }
        }
        return nullptr;
    }
    EventContext *findOuter(TagId tag) {
        auto *start = outer;
        while (start) {
            if (start->tag().contain(tag)) {
                return start;
            }
            start = start->outer;
        }
        return nullptr;
    }
    EventContext *findLine(int line) {
        for (EventContext ctx = enter(); ctx.has(); ctx.next()) {
            if (ctx.contains(line)) {
                if (ctx.hasChild()) {
                    return ctx.findLine(line - ctx.counter.line);
                } else {
                    return ctx.copy();
                }
            }
        }
        return nullptr;
    }
    EventContext *getOuter(int count) {
        EventContext *current = outer;
        while (current && --count) {
            current = current->outer;
        }
        if (count != 0) {
            return nullptr;
        }
        return current;
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
    bool contains(int line);

    template <typename Type>
    inline Type *cast() { return (Type *) element; }
    bool compare(EventContext *rvalue) {
        if (rvalue == nullptr) {
            return false;
        }
        if (element == rvalue->element) {
            if (outer) {
                return outer->compare(rvalue->outer);
            }
            return rvalue->outer == nullptr;
        }
        return false;
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
    void gutter();
};

#endif //GEDITOR_EVENT_H
