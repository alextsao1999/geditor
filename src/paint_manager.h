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
class EventContext;
class Painter {
private:
    HDC m_HDC{};
    EventContext *context;
    Offset offset;
public:
    explicit Painter(HDC m_HDC, EventContext *context);

    void drawLine(int x1, int y1, int x2, int y2) {
        MoveToEx(m_HDC, x1 + offset.x, y1 + offset.y, nullptr);
        LineTo(m_HDC, x2 + offset.x, y2 + offset.y);
    };

    void drawVerticalLine(int x, int y, int length) {
        drawLine(x, y, x, y + length);
    }

    void drawHorizentalLine(int x, int y, int length) {
        drawLine(x, y, x + length, y);
    }

    void drawRect(int x1, int y1, int x2, int y2) {
        drawLine(x1, y1, x2, y1);
        drawLine(x2, y1, x2, y2);
        drawLine(x1, y2, x2, y2);
        drawLine(x1, y1, x1, y2);
    }

    void fillRect() {
        RECT rect;
        HBRUSH brush = CreateBrushIndirect(nullptr);
        FillRect(m_HDC, &rect, brush);
    }
    void drawText(int x, int y, const GChar *str, int count) {
        // SetBkMode(m_HDC, TRANSPARENT);
        SetTextColor(m_HDC, RGB(0, 0, 0));
        TextOut(m_HDC, offset.x + x, offset.y + y, str, count);
    }
};

class TextMeter {
    virtual int meter(GChar *text, int length) { return 0; }
};

class PaintManager {
public:
    HDC m_HDC;
public:
    PaintManager() : m_HDC(CreateCompatibleDC(nullptr)) {

    }
    virtual bool isViewport(Rect &&rect) { return isViewport(rect); }
    virtual bool isViewport(Rect &rect) { return false; }
    virtual Painter getPainter(EventContext *ctx) { return Painter(m_HDC, ctx); }
    virtual TextMeter getTextMeter() { return {}; }
    virtual Size getViewportSize() { return {0, 0}; };

    bool copy(HDC hdc, int nWidth, int nHeight) {
        return (bool) BitBlt(hdc, 0, 0, nWidth, nHeight, m_HDC, 0, 0, SRCCOPY);
    }

};


#endif //TEST_PAINT_MANAGER_H
