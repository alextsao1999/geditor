//
// Created by Alex on 2019/6/26.
//

#ifndef _PAINT_MANAGER_H
#define _PAINT_MANAGER_H

#include <iostream>
#include <map>
#include <memory>
#include <SkBitmap.h>
#include <SkCanvas.h>
#include <SkString.h>
#include <SkTypeface.h>
#include <SkParse.h>
#include <SkFontStyle.h>
#include <SkFontMgr.h>
#include <SkFont.h>
#include <SkImageDecoder.h>
#include <SkBlurDrawLooper.h>
#include <SkLayerDrawLooper.h>
#include <SkBlurMaskFilter.h>
#include <SkBlurMask.h>
#include <SkDashPathEffect.h>
#include <SkGradientShader.h>
#include "SkTextBlob.h"
#include "common.h"
#include "layout.h"

struct EventContext;
class GEditorData;
class Document;
typedef SkColor GColor;
typedef SkRect GRect;
typedef SkPath GPath;
typedef SkPaint GPaint;
typedef SkPoint GPoint;
typedef SkScalar GScalar;
struct Size {
    int width;
    int height;
    Size(int width, int height) : width(width), height(height) {}
};
enum {
    StyleDeafault,
    StyleBorder,
    StyleTableFont,
    StyleTableBorder,
    StyleTableSelected,

    StyleSelectedFont,
    StyleSelectedBackground,

    StyleErrorFont,
    StyleDeafaultFont,
    StyleOperatorFont,
    StyleStringFont,
    StyleNumberFont,
    StyleKeywordFont,
    StyleFunctionFont,

    StyleControlLine,
};
class GStyle {
public:
    enum StyleType {
        StyleFill,
        StyleStroke,
        StyleFillAndStroke
    };
    enum FontType {
        kNormal = 0,
        kBold   = 0x01,
        kItalic = 0x02,
        kBoldItalic = 0x03
    };
    enum CapType {
        CapButt,
        CapRound,
        CapSquare
    };
    SkPaint m_paint;
    HFONT fFont = nullptr;
    inline SkPaint &paint() { return m_paint; }
    inline SkPaint *operator->() { return &m_paint; }
    inline operator SkPaint&() { return m_paint; }
    inline void reset() { m_paint.reset(); }
    inline void setStyle(StyleType type) { m_paint.setStyle((SkPaint::Style) type); }
    inline StyleType getStyle() { return (StyleType) m_paint.getStyle(); }
    inline void setWidth(GScalar w) { m_paint.setStrokeWidth(w); }
    inline GScalar getWidth() { return m_paint.getStrokeWidth(); }
    inline void setAlpha(uint8_t a) { m_paint.setAlpha(a); }
    inline void setStrokeCap(CapType t) { m_paint.setStrokeCap((SkPaint::Cap) t); }
    inline void setColor(GColor color) { m_paint.setColor((SkColor) color); }
    inline GColor getColor() { return m_paint.getColor(); }
    inline void setTextEncoding(SkPaint::TextEncoding encoding) { m_paint.setTextEncoding(encoding); }
    inline void setTextSize(GScalar scalar) { m_paint.setTextSize(scalar); }
    inline void setFont(const char *name, FontType type = kNormal) {
        LOGFONTA lf;
        lf.lfHeight = (int) (getTextSize());
        lf.lfWidth = 0;
        lf.lfEscapement = 0;
        lf.lfOrientation = 0;
        lf.lfWeight = 5;
        lf.lfItalic = 0;
        lf.lfUnderline = 0;
        lf.lfStrikeOut = 0;
        lf.lfCharSet = ANSI_CHARSET;
        lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfQuality = PROOF_QUALITY;
        lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
        strcpy(lf.lfFaceName, name);
        fFont = CreateFontIndirectA(&lf);
        m_paint.setTypeface(SkTypeface::CreateFromName(name, (SkTypeface::Style) type));
    }
    inline void setAntiAlias(bool aa) { m_paint.setAntiAlias(aa); }
    inline void setFakeBoldText(bool b) { m_paint.setFakeBoldText(b); }
    inline GScalar getTextSize() { return m_paint.getTextSize(); }
    inline GScalar measureText(const void* text, size_t length) { return m_paint.measureText(text, length); }
    inline int countText(const void *text, size_t bytelength) { return m_paint.countText(text, bytelength); }
    inline int getTextWidths(const void* text, size_t byteLength, GScalar widths[],
                             GRect bounds[] = NULL) {
        return m_paint.getTextWidths(text, byteLength, widths, bounds);
    }
    inline void setLinear(GColor clr1, GColor clr2, GPoint p1 = {0, 0}, GPoint p2 = {3, 3}) {
        SkPoint points[2] = {p1, p1};
        SkColor colors[2] = {clr1, clr2};
        m_paint.setShader(
                SkGradientShader::CreateLinear(
                        points, colors, NULL, 2, SkShader::TileMode::kClamp_TileMode, 0, NULL))->unref();

    }
    inline void setBlur(GColor color, GScalar sigma, GScalar x, GScalar y) {
        m_paint.setLooper(SkBlurDrawLooper::Create(color, sigma, x, y))->unref();
    }
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
        paint.setTextSize(12);
        paint.setTextEncoding(SkPaint::TextEncoding::kUTF16_TextEncoding);
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        add(StyleTableFont, paint);

        paint.reset();
        paint.setStyle(GStyle::StyleStroke);
        paint.setColor(SkColorSetRGB(148, 148, 148));
        add(StyleTableBorder, paint);

        paint.reset();
        paint.setStyle(GStyle::StyleFillAndStroke);
        paint.setColor(SkColorSetRGB(204, 226, 254));
        add(StyleTableSelected, paint);

        paint.setColor(SK_ColorWHITE);
        paint.setStyle(GStyle::StyleFill);
        add(StyleSelectedFont, paint);

        paint.reset();
        paint.setStyle(GStyle::StyleFillAndStroke);
        paint.setColor(SkColorSetRGB(204, 226, 254));
        add(StyleSelectedBackground, paint);

        paint.reset();
        paint.setTextSize(14);
        paint.setFont("宋体");
        paint.setTextEncoding(SkPaint::TextEncoding::kUTF16_TextEncoding);
        //paint.setFakeBoldText(true);
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        //paint->setLooper(getLooper());

        add(StyleDeafaultFont, paint);

        paint.setColor(SkColorSetRGB(255, 165, 0));
        add(StyleErrorFont, paint);

        paint.setColor(SK_ColorBLUE);
        add(StyleKeywordFont, paint);

        paint.setColor(SkColorSetRGB(105, 105, 105));
        add(StyleOperatorFont, paint);

        paint.setColor(SkColorSetRGB(0, 128, 128));
        add(StyleStringFont, paint);

        paint.setColor(SkColorSetRGB(138, 43, 226));
        add(StyleNumberFont, paint);

        paint.setColor(SkColorSetRGB(178, 34, 34));
        add(StyleFunctionFont, paint);

        paint.reset();
        GScalar inter[2] = {3, 2};
        paint.setColor(SK_ColorBLACK);
        paint.setAlpha(110);
        paint->setPathEffect(SkDashPathEffect::Create(inter, 2, 25))->unref();
        paint.setBlur(SK_ColorBLACK, 10, 0, 2);
        add(StyleControlLine, paint);

    }
    static SkColor ParseColor(const char *str) {
        SkColor color;
        SkParse::FindColor(str, &color);
        return color;
    }
    static SkDrawLooper *getLooper() {
        SkLayerDrawLooper::Builder builder; // SkLayerDrawLooper的内部类
        SkLayerDrawLooper::LayerInfo info;
        info.fPaintBits = SkLayerDrawLooper::kStyle_Bit | SkLayerDrawLooper::kMaskFilter_Bit;
        info.fOffset.set(0, 0);
        info.fColorMode = SkXfermode::kSrc_Mode;
        if (SkPaint* paint = builder.addLayer(info)) {
            paint->setColor(SK_ColorWHITE);
            paint->setStyle(SkPaint::kFill_Style);
            auto *mf = SkBlurMaskFilter::Create(kNormal_SkBlurStyle, SkBlurMask::ConvertRadiusToSigma(3));
            paint->setMaskFilter(mf)->unref();
        }

        info.fColorMode = SkXfermode::kSrc_Mode;
        if (auto *paint = builder.addLayerOnTop(info)) {
            paint->setStyle(SkPaint::kStroke_Style);
            paint->setStrokeWidth(1);
            paint->setColor(SK_ColorWHITE);
            paint->setAntiAlias(true);
        }
        info.fColorMode = SkXfermode::kDst_Mode;
        builder.addLayerOnTop(info);
        return builder.detachLooper();
        return SkBlurDrawLooper::Create(SK_ColorWHITE, 3, 0, 0, SkBlurDrawLooper::kAll_BlurFlag);

    }
    void add(int id, const GStyle& paint) {
        m_map.emplace(std::pair<int, GStyle>(id, paint));
    }
    void set(int id, const GStyle& paint) {
        if (m_map.count(id)) {
            m_map[id] = paint;
        } else {
            m_map.emplace(std::pair<int, GStyle>(id, paint));
        }
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
    void translate(Offset offset) {
        m_offset += offset;
    }
    void translate(GScalar x, GScalar y) {
        m_offset += Offset{SkScalarRoundToInt(x), SkScalarRoundToInt(y)};
    }
    void rotate(GScalar deg) {

    }
    void drawLine(GScalar x0, GScalar y0, GScalar x1, GScalar y1, GStyle& style) {
        auto *hPen = CreatePen(PS_SOLID, style.getWidth(), ToRGB(style.getColor()));
        auto *hOld = SelectObject(m_HDC, hPen);
        MoveToEx(m_HDC, (int) x0 + m_offset.x, (int) y0 + m_offset.y, nullptr);
        LineTo(m_HDC, (int) x1 + m_offset.x, (int) y1 + m_offset.y);
        SelectObject(m_HDC, hOld);
        DeleteObject(hPen);
    };
    void drawRect(const GRect &rect, int style);
    void drawRect(const GRect &rect, GStyle &style) {
        auto s = style.getStyle();
        SkIRect rr = rect.round();
        rr.offset(m_offset.x, m_offset.y);
        HBRUSH hBru = nullptr;
        HPEN hPen = nullptr;
        HGDIOBJ hOldBru = nullptr, hOldPen = nullptr;
        if (s == GStyle::StyleFill || s == GStyle::StyleFillAndStroke) {
            hBru = CreateSolidBrush(ToRGB(style.getColor()));
            hOldBru = SelectObject(m_HDC, hBru);
            Rectangle(m_HDC, rr.left(), rr.top(), rr.right() + 1, rr.bottom() + 1);
        }
        if (s == GStyle::StyleStroke || s == GStyle::StyleFillAndStroke) {
            hPen = CreatePen(PS_SOLID, style.getWidth(), ToRGB(style.getColor()));
            hOldPen = SelectObject(m_HDC, hPen);
            MoveToEx(m_HDC, rr.left(), rr.top(), nullptr);
            POINT pts[4];
            pts[0] = {rr.right(), rr.top()};
            pts[1] = {rr.right(), rr.bottom()};
            pts[2] = {rr.left(), rr.bottom()};
            pts[3] = {rr.left(), rr.top()};
            PolylineTo(m_HDC, pts, 4);
        }
        SelectObject(m_HDC, hOldBru);
        SelectObject(m_HDC, hOldPen);
        DeleteObject(hPen);
        DeleteObject(hBru);
    }
    void drawText(const void* text, size_t byteLength, GScalar x, GScalar y, int style = StyleDeafaultFont);
    void drawText(const void* text, size_t byteLength, GScalar x, GScalar y, GStyle &style) {
        SelectObject(m_HDC, style.fFont);
        int count = style.countText(text, byteLength);
        SetTextColor(m_HDC, ToRGB(style.getColor()));
        SetBkMode(m_HDC, TRANSPARENT);
        TextOut(m_HDC, m_offset.x + (int) x, m_offset.y + (int) y, (LPTSTR) text, count);
    }
    static inline COLORREF ToRGB(GColor color) {
        return RGB(SkColorGetR(color), SkColorGetG(color), SkColorGetB(color));
    }
    GRect bound(GScalar dx = 0, GScalar dy = 0);
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
    Canvas &operator=(const Canvas &) = delete;
    void save();
    void save(SkPaint *paint);
    void restore() {
        if (m_count) {
            m_canvas->restoreToCount(m_count);
            m_count = 0;
        }
    }
    void rotate(GScalar deg) {
        m_canvas->rotate(deg);
    }
    void translate(Offset offset) {
        m_canvas->translate(offset.x, offset.y);
    }
    void translate(GScalar x, GScalar y) {
        m_canvas->translate(x, y);
    }
    void drawLine(GScalar x0, GScalar y0, GScalar x1, GScalar y1, GStyle& style) {
        m_canvas->drawLine(x0, y0, x1, y1, style);
    }
    void drawPath(const GPath &path, GStyle &style) {
        m_canvas->drawPath(path, style);
    }
    void drawRect(const GRect &rect, int style);
    void drawRect(const GRect &rect, GStyle &style) {
        m_canvas->drawRect(rect, style);
    }
    void drawText(const void* text, size_t byteLength, GScalar x, GScalar y, int style = StyleDeafaultFont);
    void drawText(const void *text, size_t byteLength, GScalar x, GScalar y, GStyle &style) {
        m_canvas->drawText(text, byteLength, x, y + style.getTextSize(), style);
    }
    void drawPosText(const void* text, size_t byteLength, GPoint *pts, int style = StyleDeafaultFont);
    void drawPosText(const void* text, size_t byteLength, GPoint *pts, GStyle &style) {
        m_canvas->drawPosText(text, byteLength, pts, style);
    }
    inline SkCanvas *operator->() { return m_canvas; }
    inline SkCanvas &operator*() { return *m_canvas; };
    GRect bound(GScalar dx = 0, GScalar dy = 0);
};
class RenderManager {
public:
    RenderManager() = default;
    virtual ~RenderManager() = default;
    virtual void refresh() = 0;
    virtual void invalidate() = 0;
    virtual void update(GRect *rect) = 0;
    virtual void resize() = 0;
    virtual void redraw(EventContext *ctx);
    virtual Painter getPainter(EventContext *ctx) { return Painter(nullptr, nullptr); }
    virtual Canvas getCanvas(EventContext *ctx) = 0;
    virtual Offset getViewportOffset() { return {}; }
    virtual void setViewportOffset(Offset offset) = 0;
    virtual void setVertScroll(uint32_t height) = 0;
    virtual Size getViewportSize() = 0;
    virtual GRect getViewportRect() {
        Size size = getViewportSize();
        return {0, 0, SkIntToScalar(size.width), SkIntToScalar(size.height)};
    }
    virtual bool copy() = 0;

};
class WindowRenderManager : public RenderManager {
public:
    static HBITMAP CreateBitmap(int nWidth, int nHeight, void **ppBits) {
        BITMAPINFO bmi;
        memset(&bmi, 0, sizeof(bmi));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = nWidth;
        bmi.bmiHeader.biHeight = -nHeight;
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
public:
    HWND m_hWnd = nullptr;
    HDC m_hMemDC = nullptr;
    HDC m_hWndDC = nullptr;
    HBITMAP m_hBitmap = nullptr;
    SkBitmap m_bitmap;
    std::shared_ptr<SkCanvas> m_canvas;
    SkBitmap m_background;
    SkPaint m_paint;
    explicit WindowRenderManager(HWND hwnd) : m_hWnd(hwnd) {
        m_hWndDC = GetDC(hwnd);
        m_hMemDC = CreateCompatibleDC(m_hWndDC);
        WindowRenderManager::resize();
        //SkImageDecoder::DecodeFile(R"(C:\Users\Administrator\Desktop\back.jpg)", &m_background);
        //m_paint.setAlpha(30);
    }
    ~WindowRenderManager() override {
        if (m_hBitmap) DeleteObject(m_hBitmap);
    }
public:
    virtual Document &target() = 0;
    void refresh() override { InvalidateRect(m_hWnd, nullptr, true); }
    void invalidate() override { InvalidateRect(m_hWnd, nullptr, false); }
    void resize() override {
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
    Painter getPainter(EventContext *ctx) override { return Painter(m_hMemDC, ctx); }
    Canvas getCanvas(EventContext *ctx) override {
        return Canvas(ctx, new SkCanvas(m_bitmap));
    }
    void setVertScroll(uint32_t height) override {
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
    Size getViewportSize() override {
        RECT rect;
        GetClientRect(m_hWnd, &rect);
        return {rect.right - rect.left, rect.bottom - rect.top};
    }
    bool copy() override;
    void updateViewport();
    Offset getViewportOffset() override;
    void setViewportOffset(Offset offset) override;
    void update(GRect *rect) override;

};

#endif //TEST_PAINT_MANAGER_H
