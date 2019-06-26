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
    Rect(int left, int top, int right, int bottom) : left(left), top(top), right(right), bottom(bottom) {}
    void dump() {
        std::cout << "{ " << left << " , " << top << " , " << right << " , " << bottom << " }" << std::endl;
    }
};

class Painter {
public:
    virtual void draw_line(){
    };
};

class PaintManager {
public:
    virtual bool is_viewport(Rect &&rect) {
        return is_viewport(rect);
    }
    virtual bool is_viewport(Rect &rect) {
        return false;
    }

    virtual std::unique_ptr<Painter> get_painter() {
        return std::make_unique<Painter>();
    }

};


#endif //TEST_PAINT_MANAGER_H
