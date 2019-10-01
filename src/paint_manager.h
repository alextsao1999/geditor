//
// Created by Alex on 2019/6/26.
//

#ifndef _PAINT_MANAGER_H
#define _PAINT_MANAGER_H

#include <iostream>
#include <SkBitmap.h>
#include <SkCanvas.h>
#include <SkSurface.h>
#include <SkString.h>
#include "common.h"
#include "layout.h"
#define GColor COLORREF
struct Offset {
    int x = 0;
    int y = 0;
    Offset() = default;
    Offset(int x, int y) : x(x), y(y) {}
    inline Offset operator+(const Offset &offset) {
        return {x + offset.x, y + offset.y};
    }
    inline Offset operator-(const Offset &offset) {
        return {x - offset.x, y - offset.y};
    }
    inline Offset &operator+=(const Offset &offset) {
        x += offset.x;
        y += offset.y;
        return *this;
    }
    inline Offset &operator-=(const Offset &offset) {
        x -= offset.x;
        y -= offset.y;
        return *this;
    }
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
struct EventContext;

class Painter {
private:
    HDC m_HDC{};
    EventContext *m_context;
    Offset m_offset;
public:
    explicit Painter(HDC m_HDC, EventContext *context);
    ~Painter() = default;
    void drawLine(int x1, int y1, int x2, int y2) {
        MoveToEx(m_HDC, x1 + m_offset.x, y1 + m_offset.y, nullptr);
        LineTo(m_HDC, x2 + m_offset.x, y2 + m_offset.y);
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
    void setTextColor(GColor color) {
        SetTextColor(m_HDC, color);
    }
    void setBkColor(GColor color) {
        SetBkColor(m_HDC, color);
    }
    void drawText(int x, int y, const GChar *str, int count, bool transparent = true) {
        if (transparent) {
            SetBkMode(m_HDC, TRANSPARENT);
        }
        TextOut(m_HDC, m_offset.x + x, m_offset.y + y, str, count);
    }

};
class Canvas {
public:
    SkCanvas *m_canvas;
    Offset m_offset;
    EventContext *m_context;
    int m_count = 0;
    Canvas(SkCanvas *mCanvas, EventContext *context);
    ~Canvas();
    void drawLine(int x1, int y1, int x2, int y2, SkPaint &paint) {
        m_canvas->drawLine(x1, y1, x2, y2, paint);
    };
    void drawRect(int x1, int y1, int x2, int y2, SkPaint &paint) {
        SkRect rect{};
        rect.set(x1, y1, x2, y2);
        m_canvas->drawRect(rect, paint);
    }
    void drawText(int x, int y, const GChar *str, int count, SkPaint &paint) {
        m_canvas->drawText(str, count * 2, x, y + paint.getTextSize(), paint);
    }
    inline SkCanvas *operator->() {
        return m_canvas;
    }
    inline SkCanvas &operator*() {
        return *m_canvas;
    };

    SkRect rect();

    SkRect size();
};

class GDITextMetrics {
public:
    HDC m_HDC{};
    explicit GDITextMetrics(HDC HDC) : m_HDC(HDC) {}
    int measure(const GChar *text, int length) {
        SIZE size;
        GetTextExtentPoint32(m_HDC, text, length, &size);
        return size.cx;
    }
    int meterChar(GChar ch) {
        return measure(&ch, 1);
    }
    int getTextIndex(const GChar *text, int length, int &x) {
        for (int i = 0; i < length; ++i) {
            int ch = meterChar(text[i]) / 2;
            int width = measure(text, i + 1);
            if (x < width - ch) {
                x = width - ch * 2;
                return i;
            }
        }
        x = measure(text, length);
        return length;
    }
};
typedef GDITextMetrics TextMetrics;

class RenderManager {
public:
    HWND m_hWnd = nullptr;
    HDC m_hMemDC = nullptr;
    HDC m_hWndDC = nullptr;
    HBITMAP m_hBitmap = nullptr;
    Offset m_offset;
    SkBitmap m_bitmap;
    std::shared_ptr<SkCanvas> canvas;

public:
    RenderManager() = default;
    explicit RenderManager(HWND hwnd)  {
        m_hWnd = hwnd;
        m_hWndDC = GetDC(hwnd);
        m_hMemDC = CreateCompatibleDC(m_hWndDC);
        resize();
    }
    ~RenderManager() {
        if (m_hBitmap)
            DeleteObject(m_hBitmap);
        ////////////////////////////////
    }
    static RenderManager *FromWindow(HWND hwnd) {
        return new RenderManager(hwnd);
    }
    virtual void refresh() {
        InvalidateRect(m_hWnd, nullptr, false);
    }
    virtual void update() {
        canvas->clear(SK_ColorWHITE);
    }
    virtual void resize() {
        RECT rect;
        GetWindowRect(m_hWnd, &rect);
        int w = rect.right - rect.left, h = rect.bottom - rect.top;
        //m_hBitmap = CreateCompatibleBitmap(m_hWndDC, rect.right - rect.left, rect.bottom - rect.top);
        void *bits = nullptr;
        m_hBitmap = createBitmap(w, h, &bits);
        HGDIOBJ hOldBitmap = SelectObject(m_hMemDC, m_hBitmap);
        DeleteObject(hOldBitmap);

        SkImageInfo info = SkImageInfo::Make(w, h, kN32_SkColorType, kPremul_SkAlphaType);
        m_bitmap.installPixels(info, bits, info.minRowBytes());
        canvas = std::make_shared<SkCanvas>(m_bitmap);
    }

    static HBITMAP createBitmap(int nWid, int nHei, void **ppBits) {
        BITMAPINFO bmi;
        memset(&bmi, 0, sizeof(bmi));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = nWid;
        bmi.bmiHeader.biHeight = -nHei;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = 0;
        HDC hdc = GetDC(NULL);
        LPVOID pBits = NULL;
        HBITMAP hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, ppBits, nullptr, 0);
        ReleaseDC(NULL, hdc);
        return hBmp;
    }

    virtual Painter getPainter(EventContext *ctx) {
        return Painter(m_hMemDC, ctx);
    }
    virtual Canvas getCanvas(EventContext *ctx) {
        return Canvas(canvas.get(), ctx);
    }
    virtual TextMetrics getTextMetrics() { return TextMetrics(m_hMemDC); }
    inline Offset getViewportOffset() { return m_offset; }
    virtual void setViewportOffset(Offset offset) {
        m_offset = offset;
    }
    virtual void updateViewport(LayoutManager *layoutManager) {
        Offset offset;
        offset.x = GetScrollPos(m_hWnd, SB_HORZ);
        offset.y = GetScrollPos(m_hWnd, SB_VERT);
        Size size = getViewportSize();
        // 根据整体画布的宽高来确定显示的偏移
        SCROLLINFO info;
        GetScrollInfo(m_hWnd, SB_VERT, &info);
        //info.nPage
        auto realWidth = (float) (layoutManager->getWidth() - size.width + 20);
        auto realHeight = (float) (layoutManager->getHeight() - size.height + 20);
        if (realWidth < 0.0) {
            realWidth = 0.0;
        }
        if (realHeight < 0.0) {
            realHeight = 0.0;
        }
        m_offset.x = (int) (realWidth * ((float) offset.x / 100));
        m_offset.y = (int) (realHeight * ((float) offset.y / 100));
        refresh();
    }
    virtual Size getViewportSize() {
        RECT rect;
        GetWindowRect(m_hWnd, &rect);
        return {rect.right - rect.left, rect.bottom - rect.top};
    }
    bool copy(HDC hdc, int nWidth, int nHeight) {
        return (bool) BitBlt(hdc, 0, 0, nWidth, nHeight, m_hMemDC, 0, 0, SRCCOPY);
    }

};


#endif //TEST_PAINT_MANAGER_H
