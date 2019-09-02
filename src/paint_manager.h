//
// Created by Alex on 2019/6/26.
//

#ifndef _PAINT_MANAGER_H
#define _PAINT_MANAGER_H

#include <memory>
#include <iostream>
#include "common.h"

struct Offset {
    int x = 0;
    int y = 0;
    Offset() = default;
};

struct Rect {
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
    Rect() = default;
    Rect(int left, int top, int width, int height) : left(left), top(top), right(left + width), bottom(top + height) {}
    Rect(Offset offset, int width, int height) : left(offset.x), top(offset.y), right(offset.x + width), bottom(offset.y + height) {}
    inline int width() const { return bottom - top; }
    inline int height() const { return right - left; }
    void dump() {
        // std::cout << "{ " << m_left << " , " << m_top << " , " << right << " , " << bottom << " }" << std::endl;
        std::cout << "{ x: " << left << " , y: " << top << " , w: " << right - left << " , h: " << bottom - top << " }" << std::endl;
    }
};
struct Size {
    int width;
    int height;
    Size(int width, int height) : width(width), height(height) {}
};

class Painter {
private:
    HDC m_HDC{};
public:
    explicit Painter(HDC hdc) : m_HDC(hdc) {}
    void drawLine() {

    };
};

class TextMeter {
    virtual int meter(GChar *text, int length) { return 0; }
};

class PaintManager {
private:
    HDC m_HDC{};
public:
    PaintManager() : m_HDC(CreateCompatibleDC(nullptr)) {

    }
    virtual bool isViewport(Rect &&rect) { return isViewport(rect); }
    virtual bool isViewport(Rect &rect) { return false; }
    virtual Painter getPainter() { return Painter(m_HDC); }
    virtual TextMeter getTextMeter() { return {}; }
    virtual Size getViewportSize() { return {0, 0}; };

    bool copy(HDC to) {
        return (bool) BitBlt(to, 0, 0, 0, 0, m_HDC, 0, 0, SRCCOPY);
    }

};


#endif //TEST_PAINT_MANAGER_H
