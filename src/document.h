//
// Created by Administrator on 2019/6/25.
//

#ifndef TEST_DOCUMENT_H
#define TEST_DOCUMENT_H

#include <vector>
#include "paint_manager.h"
#include "command_queue.h"
enum class Display {
    None,
    Inline,
    Block,
};

struct Context {
    PaintManager *paintManager;
    CommandQueue *queue;
};
struct Element {
    Element *parent{};

    // 将所有element链接到链表中
    virtual int get_left() {
        return 0;
    };
    virtual int get_top() {
        return 0;
    };
    virtual int get_right() {
        return 0;
    };
    virtual int get_bottom() {
        return 0;
    };
    Rect get_rect() {
        return {get_left(), get_top(), get_right(), get_bottom()};
    }
    // 子元素
    virtual std::vector<Element *> *get_list() { return nullptr; };
    bool is_viewport(Context *context) {
        return context->paintManager->is_viewport(get_rect());
    }
    virtual void mouse_enter(Context *context) {}
    virtual void mouse_move(Context *context, int x, int y) {}
    virtual void mouse_leave(Context *context) {}
    virtual void left_click(Context *context, int x, int y) {}
    virtual void left_double_click(Context *context, int x, int y) {}
    virtual void right_click(Context *context, int x, int y) {}
    virtual void right_double_click(Context *context, int x, int y) {}
    virtual void select(Context *context) {}
    virtual void unselect(Context *context) {}
    virtual void input(Context *context, const char *string) {}
    virtual Element *copy() { return nullptr; }
    virtual void undo(Context *context, Command command) {}
};
class RelativeElement : public Element {
protected:
    // 上一个同级相对布局元素
    RelativeElement *_last{};
    // 下一个同级相对布局元素
    RelativeElement *_next{};
public:
    RelativeElement() = default;
    explicit RelativeElement(RelativeElement *_last) : _last(_last) {}
    RelativeElement(RelativeElement *_last, RelativeElement *_next) : _last(_last), _next(_next) {}
    int get_left() override {
        if (_last != nullptr) {
            if (_last->get_display() == Display::Inline) {
                return _last->get_right();
            }
            if (_last->get_display() == Display::Block && parent != nullptr) {
                return parent->get_left();
            }
        }
        return 0;
    }
    int get_top() override {
        if (_last != nullptr) {
            if (_last->get_display() == Display::Inline && _last->parent != nullptr) {
                return _last->parent->get_bottom();
            }
            if (_last->get_display() == Display::Block) {
                return _last->get_bottom();
            }
        } else {
            if (parent != nullptr) {
                return parent->get_top();
            }
        }
        return 0;
    }
    int get_right() override {
        return get_top() + get_width();
    }
    int get_bottom() override {
        return get_top() + get_height();
    }
    virtual Display get_display() {
        return Display::None;
    };
    virtual int get_width() {
        return 0;
    };
    virtual int get_height() {
        return 0;
    };
};
class InlineRelativeElement : public RelativeElement {
public:
    using RelativeElement::RelativeElement;
    int get_left() override {
        if (_last != nullptr) {
            return _last->get_right();
        }
        return 0;
    }
    int get_top() override {
        if (_last != nullptr) {
            return _last->get_top();
        } else {
            if (parent != nullptr) {
                return parent->get_top();
            }
        }
        return 0;
    }
    int get_right() override {
        return get_top() + get_width();
    }
    int get_bottom() override {
        return get_top() + get_height();
    }
    virtual int get_width() {
        return 0;
    };
    virtual int get_height() {
        return 0;
    };

    Display get_display() override {
        return Display::Inline;
    }
};
class BlockRelativeElement : public RelativeElement {
public:
    using RelativeElement::RelativeElement;
    int get_left() override {
        if (parent != nullptr) {
            return parent->get_left();
        }
        return 0;
    }
    int get_top() override {
        if (_last != nullptr) {
            return _last->get_bottom();
        } else {
            if (parent != nullptr) {
                return parent->get_top();
            }
        }
        return 0;
    }
    int get_right() override {
        return get_top() + get_width();
    }
    int get_bottom() override {
        return get_top() + get_height();
    }
    virtual int get_width() {
        return 0;
    };
    virtual int get_height() {
        return 0;
    };
    Display get_display() override {
        return Display::Block;
    }

};
class AbsoluteElement : public Element {
    int left{};
    int top{};
    int width{};
    int height{};
public:
    AbsoluteElement(int left, int top, int width, int height) : left(left), top(top), width(width), height(height) {}
private:
    int get_left() override {
        if (parent != nullptr) {
            return parent->get_left() + left;
        }
        return left;
    }
    int get_top() override {
        if (parent != nullptr) {
            return parent->get_top() + top;
        }
        return top;
    }
    int get_right() override {
        return get_left() + width;
    }
    int get_bottom() override {
        return get_top() + height;
    }
};

class Document {
    std::vector<Element *> _buffer;

};


#endif //TEST_DOCUMENT_H
