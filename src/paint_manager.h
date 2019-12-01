//
// Created by Alex on 2019/6/26.
//

#ifndef _PAINT_MANAGER_H
#define _PAINT_MANAGER_H

#include <iostream>
#include <map>
#include <SkBitmap.h>
#include <SkCanvas.h>
#include <SkString.h>
#include <SkTypeface.h>
#include <SkParse.h>
#include "common.h"
#include "layout.h"
struct EventContext;
class GEditorData;
typedef SkColor GColor;
typedef SkRect GRect;
typedef SkPath GPath;
typedef SkPaint GPaint;
typedef SkPoint GPoint;
typedef SkScalar GScalar;

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
struct Size {
    int width;
    int height;
    Size(int width, int height) : width(width), height(height) {}
};
enum {
    StyleDeafault,
    StyleBorder,
    StyleTableBorder,
    StyleTableBorderSelected,

    StyleErrorFont,
    StyleDeafaultFont,
    StyleOperatorFont,
    StyleStringFont,
    StyleNumberFont,
    StyleKeywordFont,
    StyleTableFont,
};
class GStyle {
public:
    enum StyleType {
        kFill_Style,            //!< fill the geometry
        kStroke_Style,          //!< stroke the geometry
        kStrokeAndFill_Style,   //!< fill and stroke the geometry
    };
    enum FontType {
        kNormal = 0,
        kBold   = 0x01,
        kItalic = 0x02,
        kBoldItalic = 0x03
    };
    SkPaint m_paint;
    HGDIOBJ fObject = nullptr;
    HFONT fFont = nullptr;
    ~GStyle() {
        DeleteObject(fObject);
        DeleteObject(fFont);
    }
    explicit operator SkPaint() { return m_paint; }
    inline SkPaint *operator->() { return &m_paint; }
    inline SkPaint &paint() { return m_paint; }
    inline void attach(HDC hdc) {
        if (m_paint.getStyle() == SkPaint::kStroke_Style) {
            fObject = CreatePen(PS_SOLID, 1, m_paint.getColor());
        } else {
            fObject = CreateSolidBrush(m_paint.getColor());
        }

        auto hOld = SelectObject(hdc, fObject);
        DeleteObject(hOld);
        SelectObject(hdc, fFont);
        printf("fFont %p\n", fFont);
    }
    inline void reset() { m_paint.reset(); }
    inline void setStyle(StyleType type) { m_paint.setStyle((SkPaint::Style) type); }
    inline void setColor(GColor color) { m_paint.setColor((SkColor) color); }
    inline void setTextSize(GScalar scalar) { m_paint.setTextSize(scalar); }
    inline void setTextEncoding(SkPaint::TextEncoding encoding) { m_paint.setTextEncoding(encoding); }
    inline void setFont(const char *name, FontType type) {
        m_paint.setTypeface(SkTypeface::CreateFromName(name, (SkTypeface::Style) type));
    }
    inline void setAntiAlias(bool aa) { m_paint.setAntiAlias(aa); }
    inline void setFakeBoldText(bool b) { m_paint.setFakeBoldText(b); }
    inline GScalar getTextSize() { return m_paint.getTextSize(); }
    inline GScalar measureText(const void* text, size_t length) { return m_paint.measureText(text, length); }
    inline int countText(const void *text, size_t bytelength) { return m_paint.countText(text, bytelength); }
    inline int getTextWidths(const void* text, size_t byteLength, SkScalar widths[],
                             SkRect bounds[] = NULL) { return m_paint.getTextWidths(text, byteLength, widths, bounds); }

};
class StyleManager {
private:
    std::map<int, GStyle> m_map;
public:
    StyleManager() {
        GStyle paint;
        paint.reset();
        add(StyleDeafault, paint);

        paint.reset();
        paint.setColor(SK_ColorLTGRAY);
        add(StyleBorder, paint);

        paint.reset();
        paint.setStyle(GStyle::kStroke_Style);
        paint.setColor(SK_ColorLTGRAY);
        add(StyleTableBorder, paint);

        paint.reset();
        paint.setStyle(GStyle::kStrokeAndFill_Style);
        paint.setColor(SK_ColorLTGRAY);
        add(StyleTableBorderSelected, paint);

        paint.reset();
        paint.setFont("DengXian", GStyle::kNormal);
        paint.setTextSize(20);
        paint.setTextEncoding(SkPaint::TextEncoding::kUTF16_TextEncoding);
        paint.setFakeBoldText(true);
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        add(StyleDeafaultFont, paint);

        paint.setColor(ParseColor("#FF8C00"));
        add(StyleErrorFont, paint);

        paint.setColor(ParseColor("#0000FF"));
        add(StyleKeywordFont, paint);

        paint.setColor(ParseColor("#8B7D7B"));
        add(StyleOperatorFont, paint);

        paint.setColor(ParseColor("#008B8B"));
        add(StyleStringFont, paint);

        paint.setColor(ParseColor("#9400D3"));
        add(StyleNumberFont, paint);

        paint.reset();
        paint.setFont("DengXian", GStyle::kNormal);
        paint.setTextSize(18);
        paint.setTextEncoding(SkPaint::TextEncoding::kUTF16_TextEncoding);
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        add(StyleTableFont, paint);

    }
    static SkColor ParseColor(const char *str) {
        SkColor color;
        SkParse::FindColor(str, &color);
        return color;
    }
    void add(int id, const GStyle& paint) {
        m_map.emplace(std::pair<int, GStyle>(id, paint));
    }
    GStyle &get(int id) {
        return m_map[id];
    }
    bool has(int id) {
        return m_map.count(id) > 0;
    }
};
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
    void drawText(int x, int y, const GChar *str, int count) {
        SetBkMode(m_HDC, TRANSPARENT);
        TextOut(m_HDC, m_offset.x + x, m_offset.y + y, str, count);
    }
    void drawText(const void* text, size_t byteLength, GScalar x, GScalar y, int style);

};
class Canvas {
public:
    SkCanvas *m_canvas;
    Offset m_offset;
    EventContext *m_context;
    int m_count = 0;

    Canvas(EventContext *context, SkCanvas *canvas, SkPaint *paint = nullptr);
    ~Canvas();

    void restore() {
        if (m_count) {
            m_canvas->restoreToCount(m_count);
            m_count = 0;
        }
    }
    void translate(GScalar x, GScalar y) {
        m_canvas->translate(x, y);
    }

    void drawRect(const GRect &rect, int style);
    void drawText(const void* text, size_t byteLength, GScalar x, GScalar y, int style);
    inline SkCanvas *operator->() {
        return m_canvas;
    }
    inline SkCanvas &operator*() {
        return *m_canvas;
    };

    GRect bound(Offset inset = Offset());

    GRect bound(GScalar dx = 0, GScalar dy = 0);

};

class RenderManager {
public:
    HWND m_hWnd = nullptr;
    HDC m_hMemDC = nullptr;
    HDC m_hWndDC = nullptr;
    HBITMAP m_hBitmap = nullptr;
    Offset m_offset{-10, -10};
    SkBitmap m_bitmap;
    std::shared_ptr<SkCanvas> m_canvas;

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
    static RenderManager *FromWindow(HWND hwnd) { return new RenderManager(hwnd); }
    virtual void refresh() { InvalidateRect(m_hWnd, nullptr, false); }
    virtual void update() { m_canvas->clear(SK_ColorWHITE); }
    virtual void resize() {
        RECT rect;
        GetWindowRect(m_hWnd, &rect);
        int w = rect.right - rect.left, h = rect.bottom - rect.top;
        //m_hBitmap = CreateCompatibleBitmap(m_hWndDC, rect.right - rect.left, rect.bottom - rect.top);
        void *bits = nullptr;
        m_hBitmap = CreateBitmap(w, h, &bits);
        HGDIOBJ hOldBitmap = SelectObject(m_hMemDC, m_hBitmap);
        DeleteObject(hOldBitmap);

        SkImageInfo info = SkImageInfo::Make(w, h, kN32_SkColorType, kPremul_SkAlphaType);
        m_bitmap.installPixels(info, bits, info.minRowBytes());
        m_canvas = std::make_shared<SkCanvas>(m_bitmap);
        update();
    }
    virtual void redraw(GEditorData *data, EventContext &context, GRect &rect);
    virtual void redraw(EventContext *ctx);
    static HBITMAP CreateBitmap(int nWid, int nHei, void **ppBits) {
        BITMAPINFO bmi;
        memset(&bmi, 0, sizeof(bmi));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = nWid;
        bmi.bmiHeader.biHeight = -nHei;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = 0;
        HDC hdc = GetDC(nullptr);
        LPVOID pBits = nullptr;
        HBITMAP hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, ppBits, nullptr, 0);
        ReleaseDC(nullptr, hdc);
        return hBmp;
    }
    virtual Painter getPainter(EventContext *ctx) { return Painter(m_hMemDC, ctx); }
    virtual Canvas getCanvas(EventContext *ctx, SkPaint *paint) { return Canvas(ctx, m_canvas.get(), paint); }
    inline Offset getViewportOffset() { return m_offset; }
    virtual void setViewportOffset(Offset offset) { m_offset = offset; }
    virtual void updateViewport(LayoutManager *layoutManager) {
        Offset offset;
        offset.x = GetScrollPos(m_hWnd, SB_HORZ);
        offset.y = GetScrollPos(m_hWnd, SB_VERT);
        Size size = getViewportSize();
        // 根据整体画布的宽高来确定显示的偏移
        SCROLLINFO info;
        GetScrollInfo(m_hWnd, SB_VERT, &info);
        //info.nPage
        auto realWidth = (float) (layoutManager->getWidth() - size.width + 20) + 10;
        auto realHeight = (float) (layoutManager->getHeight() - size.height + 20) + 10;
        if (realWidth < 0.0) {
            realWidth = 0.0;
        }
        if (realHeight < 0.0) {
            realHeight = 0.0;
        }
        m_offset.x = (int) (realWidth * ((float) offset.x / 100)) - 10;
        m_offset.y = (int) (realHeight * ((float) offset.y / 100)) - 10;
        update();
        refresh();
    }
    virtual Size getViewportSize() {
        RECT rect;
        GetWindowRect(m_hWnd, &rect);
        return {rect.right - rect.left, rect.bottom - rect.top};
    }
    bool copy() {
        Size size = getViewportSize();
        return (bool) BitBlt(m_hWndDC, 0, 0, size.width, size.height, m_hMemDC, 0, 0, SRCCOPY);
    }
};


#endif //TEST_PAINT_MANAGER_H
