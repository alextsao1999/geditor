//
// Created by Alex on 2019/6/25.
//

#ifndef _DOCUMENT_H
#define _DOCUMENT_H

#include <vector>
#include "paint_manager.h"
#include "command_queue.h"
#include "layout.h"
#define DEFINE_EVENT(event, ...) virtual void event(Context *context, ##__VA_ARGS__) {}
enum class Display {
    None,
    Inline,
    Block,
};

struct Context {
    PaintManager *m_paintManager;
    CommandQueue *m_queue;
    LayoutManager *m_layoutManager;
};

struct Element {
    Element *m_parent{};
    virtual Rect getRect() { return {0, 0, 0, 0}; }
    virtual Rect getLogicRect() { return {0, 0, 0, 0}; }
    virtual std::vector<Element *> *children() { return nullptr; };
    bool isViewport(Context *context) { return context->m_paintManager->isViewport(getRect()); }
    DEFINE_EVENT(draw);
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

class RelativeElement : public Element {
protected:
    RelativeElement *m_prev{};
    RelativeElement *m_next{};
public:
    RelativeElement() = default;
    explicit RelativeElement(RelativeElement *_last) : m_prev(_last) {}
    RelativeElement(RelativeElement *_prev, RelativeElement *_next) : m_prev(_prev), m_next(_next) {}
    virtual Display getDisplay() { return Display::None; };
    virtual int getWidth() { return 0; };
    virtual int getHeight() { return 0; };
    virtual void setWidth(Context *context, int width) {
        context->m_layoutManager->redraw(this);
    }
    virtual void setHeight(Context *context, int height) {
        context->m_layoutManager->reflow(this);
    }
};

class InlineRelativeElement : public RelativeElement {
public:
    using RelativeElement::RelativeElement;
    Rect getRect() override {
        Rect rect = getLogicRect();
        if (m_prev != nullptr) {
            Rect prev = m_prev->getRect();
            rect.left += prev.right;
            rect.top += prev.bottom;
            rect.bottom = rect.top + getHeight();
            rect.right = rect.left + getWidth();
        } else {
            Rect parent = m_parent->getRect();
            rect.top += parent.top;
            rect.left += parent.left;
            rect.bottom = rect.top + getHeight();
            rect.right = rect.left + getWidth();
        }
       return rect;
    }
    int getWidth() override { return 0; };
    int getHeight() override { return 0; };
    Display getDisplay() override { return Display::Inline; }
};

class BlockRelativeElement : public RelativeElement {
public:
    using RelativeElement::RelativeElement;
    Rect getRect() override {
        if (m_prev != nullptr) {
            Rect prev = m_prev->getRect();
            prev.left = prev.right;
            prev.top = prev.bottom;
            prev.right = prev.left + getWidth();
            prev.bottom = prev.top + getHeight();
            return prev;
        } else {
            Rect rect = getLogicRect();
            if (m_parent != nullptr) {
                Rect base = m_parent->getRect();
                rect.top += base.top;
                rect.left += base.left;
                rect.bottom = rect.top + getHeight();
                rect.right = rect.left + getWidth();
            }
            return rect;
        }
    }
    Display getDisplay() override { return Display::Block; }
};
class AbsoluteElement : public Element {
    int left{};
    int top{};
    int width{};
    int height{};
public:
    AbsoluteElement(int left, int top, int width, int height) : left(left), top(top), width(width), height(height) {}
private:
public:
    Rect getLogicRect() override {
        return Rect(left, top, width, height);
    }
    Rect getRect() override {
        if (m_parent != nullptr) {
            Rect parent = m_parent->getRect();
            parent.left += left;
            parent.top += top;
            parent.bottom = parent.top + height;
            parent.right = parent.left + width;
            return parent;
        }
        return {0, 0, 0, 0};
    }

};

class Document {
    std::vector<Element *> _buffer;

};


#endif //TEST_DOCUMENT_H
