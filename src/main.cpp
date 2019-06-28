#include <iostream>
#include "document.h"

class TestElement : public BlockRelativeElement {
    Display display;
    int width{0};
    int height{0};
public:
    TestElement(Display display, int width, int height) : display(display), width(width), height(height) {}
    TestElement(RelativeElement *prev, int width, int height) : BlockRelativeElement(prev), width(width), height(height) {}
private:
    Display getDisplay() override {
        return display;
    }
    int getWidth() override {
        return width;
    }
    int getHeight() override {
        return height;
    }
};
int main() {
    auto *ele = new TestElement(Display::Block, 100, 200);
    auto *ele1 = new TestElement(ele, 10, 20);
    auto *ele2 = new TestElement(ele1, 10, 20);
    ele->getRect().dump();
    ele1->getRect().dump();
    ele2->getRect().dump();
    return 0;
}

