#include <iostream>
#include "document.h"

class TestElement : public BlockRelativeElement {
    Display display;
    int width{0};
    int height{0};
public:
    TestElement(Display display, int width, int height) : display(display), width(width), height(height) {}

    TestElement(RelativeElement *_last, int width, int height) : BlockRelativeElement(_last), width(width),
                                                                 height(height) {

    }

    void set_size(int width, int height) {
        this->width = width;
        this->height = height;
    }

private:
    Display get_display() override {
        return display;
    }
    int get_width() override {
        return width;
    }
    int get_height() override {
        return height;
    }
};
int main() {
    auto *ele = new TestElement(Display::Block, 100, 200);
    auto *ele1 = new TestElement(ele, 10, 20);
    auto *ele2 = new TestElement(ele1, 10, 20);
    ele->get_rect().dump();
    ele1->get_rect().dump();
    ele2->get_rect().dump();
    return 0;
}

