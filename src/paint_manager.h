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
#include <SkFontStyle.h>
#include <SkFontMgr.h>
#include <SkFont.h>
#include <SkTextBlob.h>
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

    StyleSelectedFont,
    StyleSelectedBackground,

    StyleErrorFont,
    StyleDeafaultFont,
    StyleOperatorFont,
    StyleStringFont,
    StyleNumberFont,
    StyleKeywordFont,
    StyleFunctionFont,

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
    HFONT fFont = nullptr;
    ~GStyle() {
        DeleteObject(fFont);
    }
    explicit operator SkPaint() { return m_paint; }
    inline SkPaint *operator->() { return &m_paint; }
    inline SkPaint &paint() { return m_paint; }
    inline void attach(HDC hdc) {
        HGDIOBJ obj;
        if (m_paint.getStyle() == SkPaint::Style::kStroke_Style) {
            int stroke = m_paint.getStrokeWidth() == 0 ? 1 : (int) m_paint.getStrokeWidth();
            obj = CreatePen(PS_SOLID, stroke, m_paint.getColor());
        } else {
            obj = CreateSolidBrush(m_paint.getColor());
        }
        auto hOld = SelectObject(hdc, obj);
        DeleteObject(hOld);
        SelectObject(hdc, fFont);
    }
    inline void reset() { m_paint.reset(); }
    inline void setStyle(StyleType type) { m_paint.setStyle((SkPaint::Style) type); }
    inline void setColor(GColor color) { m_paint.setColor((SkColor) color); }
    inline void setTextEncoding(SkPaint::TextEncoding encoding) { m_paint.setTextEncoding(encoding); }
    inline void setTextSize(GScalar scalar) { m_paint.setTextSize(scalar); }
    inline void setFont(const char *name, FontType type) {
        m_paint.setTypeface(SkTypeface::CreateFromName(name, (SkTypeface::Style) type));
        fFont = CreateFontA(m_paint.getTextSize(), 0, 0, 0, 0, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY,
                            DEFAULT_PITCH | FF_SWISS, "宋体");
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
        paint.setColor(SkColorSetRGB(172, 172, 172));
        add(StyleBorder, paint);

        paint.reset();
        paint.setStyle(GStyle::kStroke_Style);
        paint.setColor(SkColorSetRGB(172, 172, 172));
        add(StyleTableBorder, paint);

        paint.reset();
        paint.setStyle(GStyle::kStrokeAndFill_Style);
        paint.setColor(SK_ColorLTGRAY);
        add(StyleTableBorderSelected, paint);

        paint.reset();
        paint.setTextSize(15);
        //paint.setFont("DengXian", GStyle::kNormal);
        paint.setTextEncoding(SkPaint::TextEncoding::kUTF16_TextEncoding);
        //paint.setFakeBoldText(true);
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        add(StyleDeafaultFont, paint);

        paint.setColor(SkColorSetRGB(255, 165, 0));
        add(StyleErrorFont, paint);

        paint.setColor(SkColorSetRGB(65, 105, 225));
        add(StyleKeywordFont, paint);

        paint.setColor(SkColorSetRGB(105, 105, 105));
        add(StyleOperatorFont, paint);

        paint.setColor(SkColorSetRGB(0, 128, 128));
        add(StyleStringFont, paint);

        paint.setColor(SkColorSetRGB(138, 43, 226));
        add(StyleNumberFont, paint);

        paint.setColor(SkColorSetRGB(160, 82, 45));
        add(StyleFunctionFont, paint);

        paint.setColor(SK_ColorWHITE);
        add(StyleSelectedFont, paint);

        paint.reset();
        paint.setTextSize(15);
        //paint.setFont("DengXian", GStyle::kNormal);
        paint.setTextEncoding(SkPaint::TextEncoding::kUTF16_TextEncoding);
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        add(StyleTableFont, paint);


        paint.reset();
        paint.setStyle(GStyle::kStrokeAndFill_Style);
        paint.setColor(SK_ColorLTGRAY);
        add(StyleSelectedBackground, paint);
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
    void setBkColor(GColor color) {
        SetBkColor(m_HDC, color);
    }
    void drawText(const void* text, size_t byteLength, GScalar x, GScalar y, int style);
};
class Canvas {
public:
    SkCanvas *m_canvas;
    Offset m_offset;
    EventContext *m_context;
    int m_count = 0;

    Canvas(EventContext *context, SkCanvas *canvas, SkPaint *paint);
    Canvas(EventContext *context, SkCanvas *canvas);
    ~Canvas();
    void save();
    void save(SkPaint *paint);
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
    inline SkCanvas *operator->() { return m_canvas; }
    inline SkCanvas &operator*() { return *m_canvas; };
    GRect bound(Offset inset = Offset());
    GRect bound(GScalar dx = 0, GScalar dy = 0);
};
class RenderManager {
public:
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
    HWND m_hWnd = nullptr;
    HDC m_hMemDC = nullptr;
    HDC m_hWndDC = nullptr;
    HBITMAP m_hBitmap = nullptr;
    Offset m_viewportOffset{10, 10};
    Offset m_offset{-10, -10};
    SkBitmap m_bitmap;
    std::shared_ptr<SkCanvas> m_canvas;
    GEditorData *m_data = nullptr;
public:
    RenderManager() = default;
    explicit RenderManager(HWND hwnd, GEditorData *data) : m_hWnd(hwnd), m_data(data) {
        m_hWndDC = GetDC(hwnd);
        m_hMemDC = CreateCompatibleDC(m_hWndDC);
        resize();
    };
    ~RenderManager() {
        if (m_hBitmap)
            DeleteObject(m_hBitmap);
        ////////////////////////////////
    }
    virtual void refresh() { InvalidateRect(m_hWnd, nullptr, false); }
    virtual void invalidate() { InvalidateRect(m_hWnd, nullptr, false); }
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
    }
    virtual void redrawRect(GRect *rect);
    virtual void redraw(EventContext *ctx);
    virtual Painter getPainter(EventContext *ctx) { return Painter(m_hMemDC, ctx); }
    virtual Canvas getCanvas(EventContext *ctx, SkPaint *paint) {
        //return Canvas(ctx, m_canvas.get(), paint);
        return Canvas(ctx, new SkCanvas(m_bitmap), paint);
    }
    virtual Canvas getCanvas(EventContext *ctx) {
//        return Canvas(ctx, m_canvas.get());
        return Canvas(ctx, new SkCanvas(m_bitmap));
    }
    inline Offset getViewportOffset() { return m_offset; }
    virtual void setVertScroll(uint32_t height) {
        SCROLLINFO info;
        info.cbSize = sizeof(SCROLLINFO);
        info.fMask = SIF_ALL;
        GetScrollInfo(m_hWnd, SB_VERT, &info);
        uint32_t vHeight = getViewportSize().height;
        info.nMax = height - vHeight + 20;
        info.nPage = vHeight * (vHeight / height);

        info.cbSize = sizeof(SCROLLINFO);
        info.fMask = SIF_ALL;
        SetScrollInfo(m_hWnd, SB_VERT, &info, true);
    }
    virtual void updateViewport(LayoutManager *layoutManager) {
        Offset offset;
        offset.x = GetScrollPos(m_hWnd, SB_HORZ);
        offset.y = GetScrollPos(m_hWnd, SB_VERT);
        Size size = getViewportSize();
        // 根据整体画布的宽高来确定显示的偏移
        SCROLLINFO info;
        GetScrollInfo(m_hWnd, SB_VERT, &info);
        offset -= m_viewportOffset;
        updateBegin(m_offset, offset);
        m_offset = offset;
//        m_offset.x = (int) (realWidth * ((float) offset.x / 100));
//        m_offset.y = (int) (realHeight * ((float) offset.y / 100));
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
    void updateBegin(Offset before, Offset now);
};


#endif //TEST_PAINT_MANAGER_H
