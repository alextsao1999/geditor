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
class EventContext;

class PaintManger {
private:
    HDC m_HDC{nullptr};
public:
    PaintManger() = default;
    SkPaint m_paints;

    int m_curFont = 0;
    HFONT m_fonts[1] = {
            CreateFont(18, 0, 0, 0, 0, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                       DEFAULT_PITCH | FF_SWISS, _GT("宋体"))
    };
    enum Font {
        FontDefault,
    };

    int m_curBrush = 0;
    HBRUSH m_brushes[2] = {
            CreateSolidBrush(RGB (255, 255, 255)),
            CreateSolidBrush(RGB (255, 250, 227)),
    };
    enum Brush {
        BrushBackground,
        BrushFocusBkg,
    };

    int m_curPen = 0;
    HPEN m_pens[3] = {
            CreatePen(PS_SOLID, 1, RGB (0, 0, 0)),
            CreatePen(PS_SOLID, 1, RGB (255, 0, 0)),
            CreatePen(PS_SOLID, 1, RGB (0, 0, 255)),
    };
    enum Pen {
        PenDefault,
        PenRed,
        PenBlue,
    };

    ~PaintManger() {
        for (auto font : m_fonts) {
            DeleteObject(font);
        }
        for (auto brush : m_brushes) {
            DeleteObject(brush);
        }
        for (auto pen : m_pens) {
            DeleteObject(pen);
        }
    }

    void setFont(int nFont, HFONT hFont) {
        DeleteObject(m_fonts[nFont]);
        m_fonts[nFont] = hFont;
    }

    void setFont(int nFont, const GChar *name, int cHeight, int iQuality = DEFAULT_QUALITY, int cWidth = 0, int cWeight = 0,
                 int cEscapement = 0,
                 int cOrientation = 0) {
        DeleteObject(m_fonts[nFont]);
        //int cHeight,int cWidth,int cEscapement,int cOrientation,int cWeight,
        // DWORD bItalic,DWORD bUnderline,DWORD bStrikeOut,
        // DWORD iCharSet,DWORD iOutPrecision,DWORD iClipPrecision,
        // DWORD iQuality,
        // DWORD iPitchAndFamily,LPCWSTR pszFaceName
        m_fonts[nFont] = CreateFont(cHeight, cWidth, cEscapement, cOrientation, cWeight, 0, 0, 0, ANSI_CHARSET,
                                    OUT_DEFAULT_PRECIS,
                                    CLIP_DEFAULT_PRECIS,
                                    DEFAULT_QUALITY,
                                    DEFAULT_PITCH | FF_SWISS, name);
    }

    void useFont(int nFont) {
        SelectObject(m_HDC, m_fonts[nFont]);
        m_curFont = nFont;
    }

    void setBrush(int nBrush, GColor color) {
        if (m_brushes[nBrush]) {
            DeleteObject(m_brushes[nBrush]);
        }
        m_brushes[nBrush] = CreateSolidBrush(color);

    }
    void useBrush(int nBrush) {
        SelectObject(m_HDC, m_brushes[nBrush]);
        m_curBrush = nBrush;
    }
    HBRUSH getBrush() {
        return m_brushes[m_curBrush];
    }

    void usePen(int nPen) {
        SelectObject(m_HDC, m_pens[nPen]);
        m_curPen = nPen;
    }
};

class GDIPainter {
private:
    HDC m_HDC{};
    EventContext *m_context;
    Offset m_offset;
    PaintManger *m_object;
public:
    explicit GDIPainter(HDC m_HDC, EventContext *context, PaintManger *obj);
    ~GDIPainter() = default;
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
    void fillRect(int x1, int y1, int x2, int y2) {
        RECT rect;
        rect.left = m_offset.x + x1;
        rect.top = m_offset.y + y1;
        rect.right = m_offset.x + x2;
        rect.bottom = m_offset.y + y2;
        FillRect(m_HDC, &rect, m_object->getBrush());
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
    inline PaintManger *getObjectManager() { return m_object; }

};
typedef GDIPainter Painter;

class TextMetrics {
public:
    HDC m_HDC{};
    explicit TextMetrics(HDC HDC) : m_HDC(HDC) {}
    int meterWidth(const GChar *text, int length) {
        SIZE size;
        GetTextExtentPoint32(m_HDC, text, length, &size);
        return size.cx;
    }
    int meterHeight(const GChar *text, int length) {
        SIZE size;
        GetTextExtentPoint32(m_HDC, text, length, &size);
        return size.cy;
    }
    int meterChar(GChar ch) {
        return meterWidth(&ch, 1);
    }
    int getTextIndex(const GChar *text, int length, int &x) {
        for (int i = 0; i < length; ++i) {
            int ch = meterChar(text[i]) / 2;
            int width = meterWidth(text, i + 1);
            if (x < width - ch) {
                x = width - ch * 2;
                return i;
            }
        }
        x = meterWidth(text, length);
        return length;
    }
};

class RenderManager {
public:
    PaintManger m_object;
    HWND m_hWnd = nullptr;
    HDC m_hMemDC = nullptr;
    HDC m_hWndDC = nullptr;
    HBITMAP m_hBitmap = nullptr;
    Offset m_offset;
    SkBitmap m_bitmap;

public:
    RenderManager() = default;
    explicit RenderManager(HWND hwnd)  {
        m_hWnd = hwnd;
        m_hWndDC = GetDC(hwnd);
        m_hMemDC = CreateCompatibleDC(m_hWndDC);
        m_object.set(m_hMemDC);
        m_object.useFont(PaintManger::FontDefault);

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
        RECT rect;
        GetWindowRect(m_hWnd, &rect);
        RECT rt;
        rt.left = 0;
        rt.top = 0;
        rt.right = rect.right - rect.left;
        rt.bottom = rect.bottom - rect.top;
        m_object.useBrush(PaintManger::BrushBackground);
        FillRect(m_hMemDC, &rt, m_object.getBrush());

        SkPaint paint;
        paint.setARGB(0, 255, 0, 0);
        paint.setXfermodeMode(SkXfermode::kClear_Mode);

        SkCanvas canvas(m_bitmap);
        canvas.drawRect(SkRect::MakeXYWH(30, 30, 40, 40), paint);
        canvas.drawText("this is good", 12, SkIntToScalar(30), SkIntToScalar(30), paint);

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
    }

    HBITMAP createBitmap(int nWid, int nHei, void **ppBits) {
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
    virtual PaintManger *getPaintManager() { return &m_object; }

    virtual Painter getPainter(EventContext *ctx) {
        SkCanvas canvas(m_bitmap);
        canvas.save()
        return Painter(m_hMemDC, ctx, &m_object);
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
