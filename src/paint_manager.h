//
// Created by Alex on 2019/6/26.
//

#ifndef _PAINT_MANAGER_H
#define _PAINT_MANAGER_H

#include <memory>
#include <iostream>

struct Rect {
    int left;
    int top;
    int right;
    int bottom;
    Rect(int left, int top, int width, int height) : left(left), top(top), right(left + width), bottom(top + height) {}
    inline int width() const { return bottom - top; }
    inline int height() const { return right - left; }
    void dump() {
        std::cout << "{ " << left << " , " << top << " , " << right << " , " << bottom << " }" << std::endl;
//        std::cout << "{ x: " << left << " , y: " << top << " , w: " << right - left << " , h: " << bottom - top << " }" << std::endl;
    }
};
struct Size {
    int width;
    int height;
    Size(int width, int height) : width(width), height(height) {}
};
class Painter {
public:
    virtual void drawLine() {};

};

class PaintManager {
public:
    virtual bool isViewport(Rect &&rect) { return isViewport(rect); }
    virtual bool isViewport(Rect &rect) { return false; }
    virtual Painter *getPainter() { return nullptr; }
    virtual Size getViewportSize() {
        return {0, 0};
    };

};


#endif //TEST_PAINT_MANAGER_H
