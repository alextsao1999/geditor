//
// Created by Alex on 2019/6/25.
//

#ifndef _DOCUMENT_H
#define _DOCUMENT_H

#include <queue>
#include <mutex>
#include <sstream>

#include "common.h"
#include "event.h"
#include "client.h"
#include "parser.h"
#include "margin.h"
#include <lalr/Parser.hpp>
#include <lalr/GrammarCompiler.hpp>

#define DEFINE_EVENT(EVENT, ...) virtual void EVENT(EventContext &context, ##__VA_ARGS__) {}
#define DEFINE_EVENT2(EVENT) \
    virtual void EVENT(EventContext &context, int x, int y) { \
        EventContext ctx = this->onDispatch(context, x, y);         \
        context_method(ctx, EVENT, x, y);                      \
    }
#define for_context(new_ctx, ctx) for (auto new_ctx = (ctx).enter(); new_ctx.has(); new_ctx.next())
#define context_on(ctx, method, ...) {auto &&__ctx = (ctx);if (!__ctx.empty())__ctx.current()->on##method(__ctx, ##__VA_ARGS__);}
#define context_on_ptr(ctx, method, ...) ((ctx) ? ((*(ctx)).current()->method(*(ctx), ##__VA_ARGS__)) : void(0))
#define context_method(context, event, ...) ((context).empty() ? void(0) : (context).current()->event(context, ##__VA_ARGS__))
// Root 无 父元素
class Root {
public:
    virtual ~Root() = default;

    ///////////////////////////////////////////////////////////////////
    virtual void dump() {}
    virtual void free() {}
    virtual Tag getTag(EventContext &context) { return TAG("Element"); }
    // 获取对于父元素的相对偏移
    virtual Offset getLogicOffset() { return {0, 0}; }
    // 获取实际的偏移
    virtual Offset getOffset(EventContext &context) { return {0, 0}; }
    virtual Offset getCaretOffset(EventContext &context) { return {0, 0}; }
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
    typedef SkCanvas *Drawable;
    virtual void onDraw(EventContext &context, Drawable canvas);
    virtual void onRedraw(EventContext &context);
    virtual void onRemove(EventContext &context);
    virtual bool onTimer(EventContext &context, int id) { return false; }
    /////////////////////////////////////////
};

// Element 有 父元素
class Element : public Root {
    friend Document;
public:
    enum NotifyType {
        None,
        Update,
    };
    using ostream = std::basic_ostream<GChar>;
    using istream = std::basic_istream<GChar>;
    Element() = default;
    Offset getOffset(EventContext &context) override;
    virtual int getChildCount() {
        int count = 0;
        Element *start = getHead();
        while (start != nullptr) {
            count++;
            start = start->getNext();
        }
        return count;
    }
    virtual Element *getHead() { return nullptr; }
    virtual Element *getTail() { return nullptr; }
    virtual void setHead(Element *ele) {}
    virtual void setTail(Element *ele) {}
    virtual Element *getNext() { return nullptr; }
    virtual Element *getPrev(){ return nullptr; }
    virtual bool onCanEnter(EventContext &context) { return getHead() != nullptr; }
    virtual bool isHead(EventContext &context) { return getPrev() == nullptr; }
    virtual bool isTail(EventContext &context) { return getNext() == nullptr; }
    virtual Element *onNext(EventContext &context) { context.index++;return getNext(); }
    virtual Element *onPrev(EventContext &context){ context.index--;return getPrev(); }
    virtual void onEnter(EventContext &context, EventContext &enter, int idx) {
        if (idx >= 0) {
            enter.index = 0;
            enter.element = getHead();
            for (int j = 0; j < idx; ++j) {
                enter.next();
            }
        } else {
            enter.index = -1;
            enter.element = getTail();
            if (enter.element) {
                enter.counter.increase(&enter, getLineNumber() - enter.element->getLineNumber());
            }
            for (int j = 0; j < -idx - 1; ++j) {
                enter.prev();
            }
        }
    }
    Element *getNextCount(int count) {
        Element *next = this;
        while (count-- && next) {
            next = next->getNext();
        }
        return next;
    }
    Element *getPrevCount(int count) {
        Element *prev = this;
        while (count-- && prev) {
            prev = prev->getPrev();
        }
        return prev;
    }
    virtual void setNext(Element *next){}
    virtual void setPrev(Element *prev){}
    virtual void setLogicOffset(Offset offset) {}
    virtual Display getDisplay(EventContext &context) { return DisplayNone; };
    // Display为Custom的时候才会调用这个方法
    virtual void onReflow(LayoutArgs()) {}
    virtual void onRelayout(EventContext &context, LayoutManager *sender) {
        EventContext ctx = context.enter();
        if (!ctx.empty()) {
            Offset offset;
            ctx.current()->onEnterReflow(ctx, offset);
            sender->reflow(ctx, true, offset);
        }
    }
    DEFINE_EVENT(onFocus);
    DEFINE_EVENT(onBlur, EventContext *focus, bool force);
    DEFINE_EVENT(onKeyDown, int code, int status);
    DEFINE_EVENT(onKeyUp, int code, int status);
    DEFINE_EVENT2(onMouseMove);
    DEFINE_EVENT2(onMouseHover);
    DEFINE_EVENT2(onMouseLeave);
    DEFINE_EVENT2(onLeftButtonUp);
    DEFINE_EVENT2(onLeftButtonDown);
    DEFINE_EVENT2(onRightButtonUp);
    DEFINE_EVENT2(onRightButtonDown);
    DEFINE_EVENT2(onLeftDoubleClick);
    DEFINE_EVENT2(onRightDoubleClick);
    DEFINE_EVENT(onInputChar, SelectionState state, int ch);
    DEFINE_EVENT(onSelect);
    DEFINE_EVENT(onUnselect);
    DEFINE_EVENT(onEnterReflow, Offset &offset);
    DEFINE_EVENT(onLeaveReflow, Offset &offset);
    DEFINE_EVENT(onFinishReflow, Offset &offset, LayoutContext &layout);
    virtual void onClone(EventContext &context) {}
    virtual void onUndo(Command command) {
        if (command.type == CommandType::AddElement) {
            Element *next = command.data.element->getNext();
            command.context->element->setNext(next);
            if (next) {
                next->setPrev(command.context->element);
            } else {
                if (command.context->outer) {
                    command.context->outer->element->setTail(command.context->element);
                }
            }
            command.context->deleteLine(command.context->current()->getLineNumber(),
                                        command.data.element->getLineNumber());
            command.data.element->free();
            command.context->reflow();
        }
        if (command.type == CommandType::DeleteElement) {
            Element *ele = command.data.element;
            command.context->current()->link(*command.context, ele, ele);
            command.context->insertLine(0, command.data.element->getLineNumber());
            command.context->reflow();
        }
        if (command.type == CommandType::ReplaceElement) {
            Element *old = command.context->element;
            command.context->element = command.data.replace.element;
            command.context->replace(old, false);

            command.context->reflow();

            command.context->setPos(command.pos);
            command.data.replace.caret->focus(false, true);
            // command.data.replace.caret->free(); // 释放caret
            command.data.replace.element->free(); // 释放替换的新元素
        }
        if (command.type == CommandType::SeparateElement) {
            link(*command.context, this, this);
            command.context->relayout();
        }
        command.context->redraw();
    }
    virtual void onUndoDiscard(Command command) {}
    virtual void onSelectionToString(EventContext &context, SelectionState state, ostream &out) {
        for_context(ctx, context) {
            SelectionState selection = ctx.getSelectionState();
            if (selection != SelectionNone) {
                ctx.current()->onSelectionToString(ctx, selection, out);
                if (selection == SelectionSelf || selection == SelectionEnd) {
                    break;
                }
            }
        }
    }
    virtual void onSelectionDelete(EventContext &context, SelectionState state) {
        for_context(ctx, context) {
            SelectionState selection = ctx.getSelectionState();
            if (selection != SelectionNone) {
                ctx.current()->onSelectionDelete(ctx, selection);
                if (selection == SelectionSelf || selection == SelectionEnd) {
                    goto leave;
                }
            }
        }
        leave:
        context.relayout();
    }
    virtual void onSelectionReplace(EventContext &context, SelectionState state, Range &range, istream &in) {
        for_context(ctx, context) {
            SelectionState selection = ctx.getSelectionState();
            if (selection != SelectionNone) {
                ctx.current()->onSelectionDelete(ctx, selection);
            }
        }
        context.relayout();
    }
    virtual void onSelectionRange(EventContext &context, SelectionState state, Range &range) {
        for_context(ctx, context) {
            SelectionState selection = ctx.getSelectionState();
            if (selection != SelectionNone) {
                ctx.current()->onSelectionRange(ctx, selection, range);
                if (selection == SelectionSelf || selection == SelectionEnd) {
                    break;
                }
            }
        }
    }
    virtual Element *onReplace(EventContext &context, Element *element) { return nullptr; }
    virtual void onNotify(EventContext &context, int type, NotifyParam param, NotifyValue other) {
        if (type == Update) {
            context.update();
        } else {
            if (context.outer)
                context.outer->notify(type, param, other);
        }
    }
    virtual EventContext onDispatch(EventContext &context, int x, int y) {
        for_context(ctx, context) {
            auto display = ctx.display();
            if (display == Display::DisplayNone) {
                continue;
            }
            if (display == Display::DisplayInline) {
                if (ctx.current()->contain(ctx, x, y)) {
                    return ctx;
                }
            } else {
                auto rect = ctx.rect();
                if (rect.fTop < (float) y && rect.fBottom > (float) y) {
                    return ctx;
                }
            }
        }
        return {};
    }
    virtual int getSelectCount(EventContext &context) {
        int count = 0;
        for_context(ctx, context) {
            count += ctx.isSelected();
        }
        return count;
    }
    virtual int getLineNumber(); // 如果加上EventContext 可以使getNext() 也加上EventContext
    int getWidth(EventContext &context) override;
    void link(EventContext &context, Element *start, Element *end) {
        Element *prev = getPrev();
        Element *next = getNext();
        if (start) {
            start->setPrev(prev);
        }
        if (end) {
            end->setNext(next);
        }
        if (prev) {
            prev->setNext(start);
        } else {
            if (context.outer)
                context.outer->element->setHead(start);
        }
        if (next) {
            next->setPrev(end);
        } else {
            if (context.outer)
                context.outer->element->setTail(end);
        }
    }
    void remove(EventContext &context) {
        Element *prev = getPrev();
        Element *next = getNext();
        if (prev) {
            prev->setNext(next);
        } else {
            if (context.outer)
                context.outer->element->setHead(prev);
        }
        if (next) {
            next->setPrev(prev);
        } else {
            if (context.outer)
                context.outer->element->setTail(next);
        }
    }

};
class LinkedElement : public Element {
protected:
    Element *m_prev = nullptr;
    Element *m_next = nullptr;
public:
    void free() override { delete this; }
    Element *getNext() override { return m_next; }
    Element *getPrev() override { return m_prev; }
    void setNext(Element *next) override { m_next = next; }
    void setPrev(Element *prev) override { m_prev = prev; }
    Element *onReplace(EventContext &context, Element *new_element) override {
        if (new_element == nullptr) {
            return this;
        }
        /* //删除元素(暂时不需要)
                if (new_element == nullptr) {
                    if (m_prev) m_prev->setNext(m_next); else context.outer->current()->setHead(m_next);
                    if (m_next) m_next->setPrev(m_prev); else context.outer->current()->setTail(m_prev);
                    return m_next;
                }
        */
        onRemove(context);
        new_element->setLogicOffset(getLogicOffset());
        int old_line = getLineNumber();
        int new_line = new_element->getLineNumber();
        int delta = new_line - old_line;
        if (delta > 0) {
            context.insertLine(old_line, delta);
        } else {
            context.deleteLine(new_line, -delta);
        }
        new_element->setPrev(m_prev);
        new_element->setNext(m_next);
        if (m_prev) {
            m_prev->setNext(new_element);
        } else {
            if (context.outer) {
                context.outer->current()->setHead(new_element);
            }
        }
        if (m_next) {
            m_next->setPrev(new_element);
        } else {
            if (context.outer) {
                context.outer->current()->setTail(new_element);
            }
        }
        return new_element;
    }
};
class RelativeElement : public LinkedElement {
    friend LayoutManager;
    friend Document;
protected:
    Offset m_offset;
public:
    using LinkedElement::LinkedElement;
    Offset getLogicOffset() override { return m_offset; }
    void setLogicOffset(Offset offset) override { m_offset = offset; }
    void dump() override {
        printf("{[%d, %d]}\n", m_offset.x, m_offset.y);
    }
};
class AbsoluteElement : public LinkedElement {
private:
    int m_left = 0;
    int m_top = 0;
    int m_right = 0;
    int m_bottom = 0;
public:
    AbsoluteElement() = default;
    AbsoluteElement(int left, int top, int right, int bottom) : m_left(left), m_top(top), m_right(right), m_bottom(bottom) {}
    Display getDisplay(EventContext &context) override { return DisplayAbsolute; }
    Offset getLogicOffset() override { return {m_left, m_top}; }
    Offset getOffset(EventContext &context) override {
        Offset offset = getLogicOffset();
        if (context.outer) {
            if (offset.x < 0) {
                offset.x += context.outer->width();
            }
            if (offset.y < 0) {
                offset.y += context.outer->height();
            }
            offset += context.outer->offset();
        }
        return offset;
    }
    int getLogicWidth(EventContext &context) override {
        int width = m_right - m_left;
        if (width < 0 && context.outer) {
            width += context.outer->width() + 1;
        }
        return width;
    }
    int getLogicHeight(EventContext &context) override {
        int height = m_bottom - m_top;
        if (height < 0) {
            height += context.outer->height() + 1;
        }
        return height;
    }
};

class DocumentManager;
// 根据Children 自动改变宽高
template <Display D = DisplayBlock>
class Container : public RelativeElement {
public:
    Element *m_head = nullptr;
    Element *m_tail = nullptr;
    int m_width = 0;
    int m_height = 0;
public:
    Container() = default;
    void free() override {
        Element *start = m_head;
        while (start != nullptr) {
            Element *next = start->getNext();
            start->free();
            start = next;
        }
        delete this;
    }
    Element *getHead() override { return m_head; }
    Element *getTail() override { return m_tail; }
    void setHead(Element *ele) override { m_head = ele; }
    void setTail(Element *ele) override { m_tail = ele; }
    void setLogicWidth(EventContext &context, int width) override { m_width = width; }
    void setLogicHeight(EventContext &context, int height) override { m_height = height; }
    int getLogicWidth(EventContext &context) override { return m_width; }
    int getLogicHeight(EventContext &context) override { return m_height; }
    int getHeight(EventContext &context) override { return getLogicHeight(context); }
    int getWidth(EventContext &context) override { return getLogicWidth(context); }
    void onFinishReflow(EventContext &context, Offset &offset, LayoutContext &layout) override {
        if (layout.lineMaxHeight) {
            m_width = offset.x;
            m_height = layout.lineMaxHeight;
        }
        if (layout.blockMaxWidth) {
            m_width = layout.blockMaxWidth;
            m_height = offset.y;
        }
    }
    Display getDisplay(EventContext &context) override { return D; }
    virtual Element *append(Element *element) {
        if (m_tail == nullptr) {
            m_tail = m_head = element;
        } else {
            m_tail->setNext(element);
            element->setPrev(m_tail);
            m_tail = element;
        }
        return element;
    }
    virtual Element *get(int index) {
        return index >= 0 ? m_head->getNextCount(index) : m_tail->getPrevCount(-index - 1);
    }
    virtual Element *replace(Element *before, Element *element) {
        if (before == nullptr) {
            return nullptr;
        }
        element->setNext(before->getNext());
        element->setPrev(before->getPrev());
        if (before == m_head) {
            m_head = element;
        } else {
            before->getPrev()->setNext(element);
        }
        if (before == m_tail) {
            m_tail = element;
        } else {
            before->getNext()->setPrev(element);
        }
        return before;
    }
    /*
    virtual Element *remove(Element *ele) {
        if (ele == nullptr) {
            return nullptr;
        }
        if (ele == m_head) {
            m_head = m_head->getNext();
            m_head->setPrev(nullptr);
        } else {
            ele->getPrev()->setNext(ele->getNext());
        }
        if (ele == m_tail) {
            m_tail = m_tail->getPrev();
            m_tail->setNext(nullptr);
        } else {
            ele->getNext()->setPrev(ele->getPrev());
        }
        return ele;
    }
    virtual Element *remove(int index) { return remove(get(index)); }
    */
    virtual Element *replace(int index, Element *element) { return replace(get(index), element); }
    virtual void append(Element *element, Element *ae){
        Element *next = element->getNext();
        if (next) {
            next->setPrev(ae);
        } else {
            m_tail = ae;
        }
        element->setNext(ae);
        ae->setPrev(element);
    }
    virtual void prepend(Element *element, Element *pe){
        Element *prev = element->getPrev();
        if (prev) {
            prev->setNext(pe);
        } else {
            m_head = pe;
        }
        element->setPrev(pe);
        pe->setNext(element);
    }
};
using BlockContainer = Container<>;
template <Display D = DisplayBlock>
class ScrollContainer : public Container<D> {
public:
    int m_contentWidth = 0;
    int m_contentHeight = 0;
    Offset m_viewport;
    ScrollContainer(int width, int height) {
        this->m_width = width;
        this->m_height = height;

    }
    void onFinishReflow(EventContext &context, Offset &offset, LayoutContext &layout) override {
        if (layout.lineMaxHeight) {
            m_contentWidth = offset.x;
            m_contentHeight = layout.lineMaxHeight;
        }
        if (layout.blockMaxWidth) {
            m_contentWidth = layout.blockMaxWidth;
            m_contentHeight = offset.y;
        }

    }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        Root::onRedraw(context);
    }
    EventContext onDispatch(EventContext &context, int x, int y) override {
        return Element::onDispatch(context, x + m_viewport.x, y + m_viewport.y);
    }
};
class Document : public Container<DisplayBlock> {
public:
    typedef int (WINAPI *CallBack)(EventContext *, int, int);
    typedef int (WINAPI *LexerHandler)(EventContext *, int, int);
    using Grammer = lalr::GrammarCompiler;
    Grammer *m_grammer = nullptr;
    LexerHandler m_lexer = nullptr;
    CallBack m_onBlur = nullptr;
    Context m_context;
    Offset m_mouse;
    Offset m_viewportOffset;
    EventContext m_root;
    EventContext m_begin;
    DocumentManager *m_manager = nullptr;
public:
    explicit Document(RenderManager *render, DocumentManager *mgr);
    Tag getTag(EventContext &context) override { return TAG("Document"); }
    DocumentManager *getDocumentManager() { return m_manager; }
    virtual string_ref getUri() { return nullptr; }
public:
    virtual Margin *margin() { return nullptr; }
    inline Context *context() { return &m_context; }
    inline TextBuffer *buffer() { return &m_context.m_textBuffer; }
    inline EventContext &root() { return m_root; }
    inline CaretManager *caret() { return &m_context.m_caretManager; }
    inline StyleManager *style() { return &m_context.m_styleManager; }
    inline RenderManager *render() { return m_context.m_renderManager; }
    inline LayoutManager *layouter() { return &m_context.m_layoutManager; }
public:
    void undo() {
        m_context.clearSelect();
        if (m_context.m_queue.has()) {
            auto command = m_context.m_queue.pop();
            if (command.type == CommandType::PushEnd) {
                CaretPos end = command.pos;
                while (m_context.m_queue.has()) {
                    command = m_context.m_queue.pop();
                    if (command.type == CommandType::PushStart) {
                        if (command.data.value == PushType::PushTypeSelect) {
                            m_context.select(command.pos, end);
                        }
                        return;
                    }
                    command.context->current()->onUndo(command);
                    command.context->free();
                }
            } else {
                command.context->current()->onUndo(command);
                command.context->free();
            }
        }
    }
    void copy() {
        auto str = getSelectionString();
        OpenClipboard(nullptr);
        EmptyClipboard();
        HGLOBAL pRes = GlobalAlloc(GMEM_MOVEABLE, (str.length() + 1) * sizeof(GChar));
        auto *pStr = (GChar *) GlobalLock(pRes);
        gstrcpy(pStr, str.c_str());
        pStr[str.length()] = _GT('\0');
        GlobalUnlock(pRes);
        SetClipboardData(CF_UNICODETEXT, pRes);
        CloseClipboard();
    }
    GString getSelectionString();
    ///////////////////////////////////////////////////////////////////
    Offset getLogicOffset() override { return {0, 0}; }
    int getLogicWidth(EventContext &context) override {
        return render()->getViewportSize().width - 50;
    }
    void setViewportOffset(Offset offset) {
        /*
        if (m_viewportOffset.y > offset.y) { // 向上
            while (m_begin.has()) {
                if (!m_begin.visible()) {
                    m_begin.next();
                    break;
                }
                m_begin.prev();
            }
        } else { // 向下
            while (m_begin.has()) {
                if (m_begin.visible()) {
                    break;
                }
                m_begin.next();
            }
        }
        */
        if (m_begin.empty()) {
            m_begin = m_root.enter();
        }
        m_viewportOffset = offset;
    }
    void onRelayout(EventContext &context, LayoutManager *sender) override {
        sender->reflow(context.enter(), true, {10, 10});
    }
    void onFinishReflow(EventContext &context, Offset &offset, LayoutContext &layout) override {
        Container::onFinishReflow(context, offset, layout);
        m_height += 200;
        render()->setVertScroll(m_height);
    }
    virtual void onLineChange(EventContext &event, int num) {}
    virtual void onContentChange(EventContext &event, CommandType type, CommandData data) {}
    virtual void onKeyCommand(KeyState key) {}
};
class MarginDocument : public Document {
public:
    SimpleParser m_parser;
    Margin m_margin;
    explicit MarginDocument(DocumentManager *mgr);
    Offset getLogicOffset() override { return {m_margin.width(), 0}; }
    void onLineChange(EventContext &event, int num) override {
        m_margin.update();
    }
    void onKeyCommand(KeyState key) override {
        auto command = m_context.m_keyMap.lookUp(key);
        EventContext *ctx = caret()->getEventContext();
        if (command == KeyCommand::CD) {
            ctx->getLineViewer().flags() |= LineFlagFold;
            ctx->redraw();
            return;
        }
        if (command == KeyCommand::CF) {
            ctx->getLineViewer().flags() |= LineFlagLineVert;
            ctx->redraw();
            return;
        }
        if (command == KeyCommand::CG) {
            ctx->getLineViewer().flags() |= LineFlagLineHorz;
            ctx->redraw();
            return;
        }
        if (command == KeyCommand::Undo) {
            undo();
        }
        if (command == KeyCommand::Paste) {
            OpenClipboard(nullptr);
            HGLOBAL pRes = GetClipboardData(CF_UNICODETEXT);
            auto *pStr = (GChar *) GlobalLock(pRes);
            GString str(pStr);
            GlobalUnlock(pRes);
            CloseClipboard();
            GIStringStream wis(str);
            context()->pushStart();
            m_parser.parse(m_context.m_caretManager.getEventContext(), wis);
            context()->pushEnd();
            m_root.redraw();
        }
        if (!m_context.hasSelection()) {
            return;
        }
        if (command == KeyCommand::Copy) {
            copy();
        }
        if (command == KeyCommand::Cut) {
            copy();
            context()->pushStart(PushTypeSelect);
            onSelectionDelete(m_root, SelectionNone);
            context()->pushEnd();
            context()->clearSelect();
            m_root.redraw();
        }

    }
    Margin *margin() override { return &m_margin; }
    void onRedraw(EventContext &context) override {
        Root::onRedraw(context);
        m_margin.draw();
    }

};
#endif //TEST_DOCUMENT_H
