//
// Created by Alex on 2019/8/4.
//

#ifndef GEDITOR_TABLE_H
#define GEDITOR_TABLE_H

#include <SkBlurDrawLooper.h>
#include <SkColorFilter.h>
#include <SkMaskFilter.h>
#include <SkDashPathEffect.h>
#include <SkBlurMaskFilter.h>
#include <SkGradientShader.h>
#include <SkImageFilter.h>
#include <SkTypeface.h>
#include <SkBlurImageFilter.h>
#include <SkPathOps.h>
#include <SkParse.h>
#include "document.h"
#include "utils.h"

class TextCaretService {
private:
    Offset m_offset;
    Offset m_move;
    EventContext* m_context = nullptr;
    int m_index = 0;
public:
    TextCaretService(const Offset& offset, EventContext* context) : m_offset(offset), m_context(context) {
        m_index = m_context->getCaretManager()->data()->index;
    }
    void setTextOffset(Offset offset) { m_offset = offset; }
    int& index() { return m_index; }
    void moveLeft() { m_index--; }
    void moveRight() { m_index++; }
    void moveToIndex(int index) { m_index = index; }
    void moveTo(Offset offset) { m_move = offset; }
    bool commit(SkPaint& paint, SkRect* bound = nullptr, int column = 0) {
        LineViewer viewer = m_context->getLineViewer(column);
        return commit(viewer.c_str(), viewer.size(), paint, bound);
    }
    bool commit(const char* text, int bytelength, SkPaint& paint, SkRect* bound = nullptr) {
        bool res; // 是否越界
        int index = m_index;
        int length = paint.countText(text, bytelength);
        auto* widths = new SkScalar[length];
        auto* rects = new SkRect[length];
        int count = paint.getTextWidths(text, bytelength, widths, rects);
        SkScalar size = paint.getTextSize();
        int width = 0;
        for (int i = 0; i < length; ++i) {
            rects[i].offset(m_offset.x + width, size + (SkScalar)m_offset.y);
            if (SkIntToScalar(m_move.x) >= rects[i].x()) {
                if (SkIntToScalar(m_move.x) > rects[i].centerX()) {
                    index = i + 1;
                } else {
                    index = i;
                }
            }
            width += widths[i];
        }
        res = index <= length; // 大于length 为越界
        while (index < 0) {
            index = length + index + 1;
            res = false; // 小于0为越界
        }
        Offset caret;
        if (index < length) {
            caret.x = rects[index].left();
            if (bound) {
                *bound = rects[index];
            }
        } else {
            index = length;
            if (length == 0) {
                caret.x = m_offset.x;
            } else {
                if (rects[length - 1].width() == 0) {
                    rects[length - 1].fRight += widths[length - 1];
                }
                caret.x = rects[length - 1].right();
                if (bound) {
                    *bound = rects[length - 1];
                }
            }
        }
        caret.y = m_offset.y;
        show(caret, index);
        delete[] widths;
        delete[] rects;
        if (res) {
            m_move = caret;
        }
        return res;
    }

    void show(Offset offset, int index) {
        CaretManager* m_caret = m_context->getCaretManager();
        m_caret->data()->index = index;
        m_caret->set(offset);
        m_caret->show();
    }
};

class TextElement : public RelativeElement {
private:
    GString m_data;
    int m_column = 0;
    int m_min = 50;
public:
    int m_width = 0;
    int m_height = 25;
    explicit TextElement(int column) : m_column(column) {}
    int getLogicHeight(EventContext &context) override { return m_height; }
    int getLogicWidth(EventContext &context) override {
        int width =
                context.getStyle(StyleTableFont).measureText((const char *) m_data.c_str(), m_data.length() * 2) + 8;
        return width > m_min ? width : m_min;
    }
    void setLogicWidth(int width) override { m_width = width; }
    void setLogicHeight(int height) override { m_height = height; }
    int getWidth(EventContext& context) override {
        if (m_width) { return m_width; }
        return getLogicWidth(context);
    }
    Display getDisplay() override { return DisplayInline; }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        canvas->drawRect(canvas.bound(), context.getStyle(StyleTableBorder));

        canvas->translate(0, context.getStyle(StyleTableFont).getTextSize());
        canvas->drawText((const char *) m_data.c_str(), m_data.length() * 2, 4, 2, context.getStyle(StyleTableFont));
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        TextCaretService service(Offset(4, 4), &context);
        service.moveTo(context.relative(x, y));
        service.commit((const char *) m_data.c_str(), m_data.length() * 2, context.getStyle(StyleTableFont));
    }
    void onMouseMove(EventContext &context, int x, int y) override {
        if (context.selecting()) {
            context.focus();
            TextCaretService service(Offset(4, 4), &context);
            service.moveTo(context.relative(x, y));
            service.commit((const char *) m_data.c_str(), m_data.length() * 2, context.getStyle(StyleTableFont));
        }
    }

    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        TextCaretService service(Offset(4, 4), &context);
        if (code == VK_LEFT) {
            service.moveLeft();
        }
        if (code == VK_RIGHT) {
            service.moveRight();
        }
        service.commit((const char *) m_data.c_str(), m_data.length() * 2, context.getStyle(StyleTableFont));
    };
    void onInputChar(EventContext &context, int ch) override {
        TextCaretService service(Offset(4, 4), &context);
        switch (ch) {
        case VK_BACK:
            if (service.index() > 0) {
                m_data.erase(service.index() - 1);
                service.moveLeft();
            }
            break;
        case VK_RETURN:
            break;
        default:
            m_data.insert(m_data.begin() + service.index(), ch);
            service.moveRight();
            break;
        }
        if (context.outer) {
            context.outer->notify(WidthChange, 0, m_column);
        }
        service.commit((const char *) m_data.c_str(), m_data.length() * 2, context.getStyle(StyleTableFont));
        context.redraw();

    }

};
class ColumnElement : public RelativeElement {
private:
    int m_column = 0;
    int m_min = 50;
public:
    int m_width = 0;
    int m_height = 25;
    explicit ColumnElement(int column) : m_column(column) {}
    int getLogicHeight(EventContext &context) override { return m_height; }
    int getLogicWidth(EventContext &context) override {
        auto line = context.getLineViewer(m_column);
        int width = (int) context.getStyle(StyleTableFont).measureText(line.str(), line.size()) + 8;
        return width > m_min ? width : m_min;
    }
    void setLogicWidth(int width) override { m_width = width; }
    void setLogicHeight(int height) override { m_height = height; }
    int getWidth(EventContext& context) override {
        if (m_width) { return m_width; }
        return getLogicWidth(context);
    }
    Display getDisplay() override {
        return DisplayInline;
    }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        canvas->drawRect(canvas.bound(), context.getStyle(StyleTableBorder));

        auto line = context.getLineViewer(m_column);
        canvas->translate(0, context.getStyle(StyleTableFont).getTextSize());
        canvas->drawText(line.c_str(), line.size(), 4, 2, context.getStyle(StyleTableFont));
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        TextCaretService service(Offset(4, 4), &context);
        service.moveTo(context.relative(x, y));
        service.commit(context.getStyle(StyleTableFont), nullptr, m_column);
    }

    void onMouseMove(EventContext &context, int x, int y) override {
        if (context.selecting()) {
            onLeftButtonDown(context, x, y);
        }
    }

    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        TextCaretService service(Offset(4, 4), &context);
        if (code == VK_LEFT) {
            service.moveLeft();
        }
        if (code == VK_RIGHT) {
            service.moveRight();
        }
        service.commit(context.getStyle(StyleTableFont), nullptr, m_column);
    };
    void onInputChar(EventContext &context, int ch) override {
        auto line = context.getLineViewer(m_column);
        TextCaretService service(Offset(4, 4), &context);
        switch (ch) {
            case VK_BACK:
                if (service.index() > 0) {
                    line.remove(service.index() - 1);
                    service.moveLeft();
                }
                break;
            case VK_RETURN:
                break;
            default:
                line.insert(service.index(), ch);
                service.moveRight();
                break;
        }
        if (context.outer) {
            context.outer->notify(WidthChange, 0, m_column);
        }
        service.commit(context.getStyle(StyleTableFont), nullptr, m_column);
        context.redraw();

    }

};
class ButtonElement : public RelativeElement {
public:
    SkBitmap bitmap;
    ButtonElement() {
        bitmap.allocN32Pixels(200, 100);
        draw(0, 0, 0,
             SkColorSetRGB(191, 191, 191), SkColorSetRGB(242, 242, 242));
    }

    Display getDisplay() override {
        return DisplayBlock;
    }
    int getLogicHeight(EventContext &context) override {
        return 100;
    }
    int getWidth(EventContext &context) override {
        return 200;
    }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        canvas->drawBitmap(bitmap, 0, 0);
/*
        SkPaint style;
        style.setColor(SK_ColorBLACK);
        style.setTextSize(15);
        style.setTextEncoding(SkPaint::kUTF16_TextEncoding);
        style.setColor(SK_ColorRED);
        canvas->drawText(m_data.c_str(), m_data.size() * 2, 4, 4 + 15, style);


        SkPaint paint;
        //paint.setColor(SK_ColorLTGRAY);
        SkPoint points[2] = {{0.0f, 0.0f}, {256.0f, 256.0f}};
        SkColor colors[2] = {SkColorSetRGB(66,133,244), SkColorSetRGB(15,157,88)};
        paint.setShader(
                SkGradientShader::CreateLinear(
                        points, colors, NULL, 2, SkShader::TileMode::kClamp_TileMode, 0, NULL));
        paint.setStrokeWidth(4);
        paint.setStyle(SkPaint::kStroke_Style);

        paint.setAntiAlias(true);
        paint.setLooper(SkBlurDrawLooper::Create(SK_ColorBLACK, 3, 0, 0));
        canvas->drawRoundRect(canvas.bound({10, 10}), 4, 4, paint);

        SkPath mystar = star(3, 35, 20);
        mystar.offset(15, 15);
        canvas->drawPath(mystar, paint);
*/
    }
    static float rad(float deg) {
        return (float) (deg * 3.1415926 / 180);
    }
    static GPath star(int num, float R, float r) {
        GPath path;
        float perDeg = 360 / num;
        float degA = perDeg / 2 / 2;
        float degB = 360 / (num - 1) / 2 - degA / 2 + degA;
        path.moveTo(
                (float) (cos(rad(degA + perDeg * 0)) * R + R * cos(rad(degA))),
                (float) (-sin(rad(degA + perDeg * 0)) * R + R));
        for (int i = 0; i < num; i++) {
            path.lineTo(
                    (float) (cos(rad(degA + perDeg * i)) * R + R * cos(rad(degA))),
                    (float) (-sin(rad(degA + perDeg * i)) * R + R));
            path.lineTo(
                    (float) (cos(rad(degB + perDeg * i)) * r + R * cos(rad(degA))),
                    (float) (-sin(rad(degB + perDeg * i)) * r + R));
        }
        path.close();
        return path;
    }
    void draw(int sigma, int posx = 0, int posy = 0, SkColor border = 0, SkColor color = 0) {

        SkRect rect{};
        bitmap.getBounds(&rect);
        rect.inset(20, 20);

        SkCanvas canvas(bitmap);
        canvas.clear(SK_ColorWHITE);

        SkPaint paint;
        paint.setStyle(SkPaint::Style::kFill_Style);
        paint.setColor(color);
        paint.setAntiAlias(true);
        paint.setLooper(SkBlurDrawLooper::Create(color, sigma, posx, posy));
        canvas.drawRoundRect(rect, 4, 4, paint);

        paint.reset();
        paint.setColor(border);
        paint.setStyle(SkPaint::Style::kStroke_Style);
        paint.setStrokeWidth(0.5);
        paint.setAntiAlias(true);
        canvas.drawRoundRect(rect, 4, 4, paint);

        SkPaint font;
        font.setColor(SK_ColorBLACK);
        char str[255] = {'\0'};
        sprintf(str, "first button");
        canvas.drawText(str, strlen(str), 90, 35, font);

    }
    EventContext *ctx{nullptr};
    void onMouseEnter(EventContext &context, int x, int y) override {
        ctx = context.copy();
        draw(1, 0, 0,
                SkColorSetRGB(161, 161, 161), SkColorSetRGB(222, 222, 222));
        context.getRenderManager()->refresh();
    }
    void onMouseLeave(int x, int y) override {
        draw(0, 0, 0,
             SkColorSetRGB(191, 191, 191), SkColorSetRGB(242, 242, 242));
        ctx->getRenderManager()->refresh();
        ctx->free();
    }
    void onMouseMove(EventContext &context, int x, int y) override {
        draw(11, 0, 0,
             SkColorSetRGB(161, 161, 161), SkColorSetRGB(222, 222, 222));
        context.getRenderManager()->refresh();
    }
    void onLeftButtonUp(EventContext &context, int x, int y) override {
        draw(1, 0, 0,
             SkColorSetRGB(161, 161, 161), SkColorSetRGB(222, 222, 222));
        context.getRenderManager()->refresh();
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        draw(11, 0, 0,
             SkColorSetRGB(166, 166, 166), SkColorSetRGB(204, 204, 204));
        context.getRenderManager()->refresh();
    }
};
class LineElement : public RelativeElement {
public:
    SkPaint paint;
    SkPaint style;
    int i = 0;
    LineElement() {
        paint.setTypeface(SkTypeface::CreateFromName("DengXian", SkTypeface::Style::kNormal));
        paint.setTextSize(18);
        paint.setTextEncoding(SkPaint::TextEncoding::kUTF16_TextEncoding);
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        paint.setLooper(SkBlurDrawLooper::Create(SK_ColorGRAY, 10, 4, 4));

    }
    int getLogicHeight(EventContext &context) override { return 25; }
    Display getDisplay() override { return DisplayLine; }
    Element *copy() override {
        return new LineElement();
    }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas(&style);

        SkPaint border;
        border.setStyle(SkPaint::Style::kStroke_Style);
        border.setColor(SK_ColorLTGRAY);
        canvas->drawRect(canvas.bound(), border);

        LineViewer viewer = context.getLineViewer();
        canvas->translate(0, paint.getTextSize());
        canvas->drawText(viewer.c_str(), viewer.size(), 4, 2, paint);

    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        TextCaretService service(Offset(4, 6), &context);
        service.moveTo(context.relative(x, y));
        service.commit(paint);
    }
    void onMouseMove(EventContext &context, int x, int y) override {
        if (context.selecting()) {
            context.focus();
            TextCaretService service(Offset(4, 6), &context);
            service.moveTo(context.relative(x, y));
            service.commit(paint);
        }
    }
    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        TextCaretService service(Offset(4, 5), &context);
        if (code == VK_LEFT) {
            service.moveLeft();
            if (!service.commit(paint)) {
                caret->prev();
                service.moveToIndex(-1);
                service.commit(paint);
            }
        }
        if (code == VK_RIGHT) {
            service.moveRight();
            if (!service.commit(paint)) {
                if (caret->next()) {
                    service.moveToIndex(0);
                    service.commit(paint);
                }
            }
        }
        if (code == VK_UP) {
            service.commit(paint);
            caret->prev();
            service.commit(paint);
        }
        if (code == VK_DOWN) {
            service.commit(paint);
            caret->next();
            service.commit(paint);
        }
    };
    void onInputChar(EventContext &context, int ch) override {
        auto* caret = context.getCaretManager();
        TextCaretService service(Offset(4, 6), &context);
        auto line = context.getLineViewer();
        switch (ch) {
            case VK_BACK:
                if (service.index() > 0) {
                    line.remove(service.index() - 1);
                    service.moveLeft();
                } else {
                    if (caret->prev()) {
                        service.moveToIndex(-1);
                        service.commit(paint);
                        context.combine(); // 因为combine要delete本对象 之后paint就不存在了 所以移动要在之前调用
                        context.reflow();
                        context.redraw();
                        return;
                    }
                }
                break;
            case VK_RETURN:
                context.copyLine();
                context.reflow();
                if (caret->next()) {
                    int idx = service.index();
                    context.getLineViewer().append(line.str() + idx, line.length() - idx);
                    line.erase(idx, line.length() - idx);
                    service.moveToIndex(0);
                }
                break;
            default:
                line.insert(service.index(), ch);
                service.moveRight();
                break;
        }
        service.commit(paint);
        context.redraw();
    }
    void onFocus(EventContext &context) override {
        // 选中
        //style.setColorFilter(SkColorFilter::CreateModeFilter(SK_ColorLTGRAY, SkXfermode::Mode::kSrcOut_Mode));
        // 设置文字
        //style.setColorFilter(SkColorFilter::CreateModeFilter(SK_ColorLTGRAY, SkXfermode::Mode::kSrcIn_Mode));
        // 设置背景
        //style.setColorFilter(SkColorFilter::CreateModeFilter(SK_ColorLTGRAY, SkXfermode::Mode::kColorDodge_Mode));
        style.setColorFilter(
                SkColorFilter::CreateModeFilter(
                        SkColorSetARGB(255, 255, 250, 227), SkXfermode::Mode::kDarken_Mode));
        context.redraw();

    }
    void onBlur(EventContext &context) override {
        style.setColorFilter(nullptr);
        context.redraw();
    }
};
class SyntaxLineElement : public RelativeElement {
public:
    SkPaint style;
    SyntaxLineElement() = default;
    int getLogicHeight(EventContext &context) override { return 25; }
    Display getDisplay() override { return DisplayLine; }
    Element *copy() override {
        return new SyntaxLineElement();
    }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas(&style);

        SkPaint border;
        border.setStyle(SkPaint::Style::kStroke_Style);
        border.setColor(SK_ColorLTGRAY);
        canvas->drawRect(canvas.bound(), border);

        LineViewer viewer = context.getLineViewer();
        canvas->translate(0, context.getStyle(StyleDeafaultFont).getTextSize());
        //canvas->drawText(viewer.c_str(), viewer.size(), 4, 2, paint);

        auto *lexer = context.getLexer(Offset(4, 2));

        while (lexer->has()) {
            Token token = lexer->next();
            GString string(token.start, token.length);
            //printf("token x:%d y:%d   %ls\n", token.offset.x, token.offset.y, string.c_str());
            canvas->drawText((char *) string.c_str(), token.length * 2, token.offset.x, token.offset.y,
                             context.getStyle(token.style));




        }

    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        TextCaretService service(Offset(4, 6), &context);
        service.moveTo(context.relative(x, y));
        service.commit(context.getStyle(StyleDeafaultFont));
    }
    void onMouseMove(EventContext &context, int x, int y) override {
        if (context.selecting()) {
            context.focus();
            TextCaretService service(Offset(4, 6), &context);
            service.moveTo(context.relative(x, y));
            service.commit(context.getStyle(StyleDeafaultFont));
        }
    }
    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        auto &paint = context.getStyle(StyleDeafaultFont);
        TextCaretService service(Offset(4, 5), &context);
        if (code == VK_LEFT) {
            service.moveLeft();
            if (!service.commit(paint)) {
                caret->prev();
                service.moveToIndex(-1);
                service.commit(paint);
            }
        }
        if (code == VK_RIGHT) {
            service.moveRight();
            if (!service.commit(paint)) {
                if (caret->next()) {
                    service.moveToIndex(0);
                    service.commit(paint);
                }
            }
        }
        if (code == VK_UP) {
            service.commit(paint);
            caret->prev();
            service.commit(paint);
        }
        if (code == VK_DOWN) {
            service.commit(paint);
            caret->next();
            service.commit(paint);
        }
    };
    void onInputChar(EventContext &context, int ch) override {
        auto* caret = context.getCaretManager();
        TextCaretService service(Offset(4, 6), &context);
        auto line = context.getLineViewer();
        switch (ch) {
            case VK_BACK:
                if (service.index() > 0) {
                    line.remove(service.index() - 1);
                    service.moveLeft();
                } else {
                    if (caret->prev()) {
                        service.moveToIndex(-1);
                        service.commit(context.getStyle(StyleDeafaultFont));
                        context.combine(); // 因为combine要delete本对象 之后paint就不存在了 所以移动要在之前调用
                        context.reflow();
                        context.redraw();
                        return;
                    }
                }
                break;
            case VK_RETURN:
                context.copyLine();
                context.reflow();
                if (caret->next()) {
                    int idx = service.index();
                    context.getLineViewer().append(line.str() + idx, line.length() - idx);
                    line.erase(idx, line.length() - idx);
                    service.moveToIndex(0);
                }
                break;
            default:
                line.insert(service.index(), ch);
                service.moveRight();
                break;
        }
        service.commit(context.getStyle(StyleDeafaultFont));
        context.redraw();
    }
    void onFocus(EventContext &context) override {
        // 选中
        //style.setColorFilter(SkColorFilter::CreateModeFilter(SK_ColorLTGRAY, SkXfermode::Mode::kSrcOut_Mode));
        // 设置文字
        //style.setColorFilter(SkColorFilter::CreateModeFilter(SK_ColorLTGRAY, SkXfermode::Mode::kSrcIn_Mode));
        // 设置背景
        //style.setColorFilter(SkColorFilter::CreateModeFilter(SK_ColorLTGRAY, SkXfermode::Mode::kColorDodge_Mode));
        style.setColorFilter(
                SkColorFilter::CreateModeFilter(
                        SkColorSetARGB(255, 255, 250, 227), SkXfermode::Mode::kDarken_Mode));
        context.redraw();

    }
    void onBlur(EventContext &context) override {
        style.setColorFilter(nullptr);
        context.redraw();
    }
};

class InlineRowElement : public Container {
public:
    explicit InlineRowElement(int column) {
        for (int i = 0; i < column; ++i) {
            append(new TextElement(i));
        }
    }
    void setLogicHeight(int height) override {
        m_height = height;
        for (auto *ele : m_index) {
            ele->setLogicHeight(m_height);
        }
    }
    void setColumnWidth(int column, int width) {
        m_index.at(column)->setLogicWidth(width);
    }
    int getColumnWidth(EventContext &context, int column) {
        return m_index.at(column)->getLogicWidth(context);
    }
    void onNotify(EventContext &context, int type, int p1, int p2) override {
        if (context.outer) {
            context.outer->notify(type, p1, p2);
        }
    }
};
class InlineTableElement : public Container {
public:
    InlineTableElement(int line, int column) : Container(DisplayInline) {
        for (int i = 0; i < line; ++i) {
            append(new InlineRowElement(column));
        }
    }

    void setColumnWidth(int column, int width) {
        for (auto *ele : m_index) {
            auto *row = (InlineRowElement *) ele;
            row->setColumnWidth(column, width);
        }
    }
    void onNotify(EventContext &context, int type, int p1, int p2) override {
        EventContext ctx = context.enter();
        int width = 0;
        while (ctx.has()) {
            auto *element = (InlineRowElement *) ctx.current();
            int cur_width = element->getColumnWidth(ctx, p2);
            if (cur_width > width)
                width = cur_width;
            ctx.next();
        }
        setColumnWidth(p2, width);
        ctx.getLayoutManager()->reflowEnter(ctx.start());
        if (context.outer) {
            context.outer->notify(WidthChange, 0, context.index);
        }
    }

};
class RowElement : public Container {
public:
    explicit RowElement(int column) : Container(DisplayLine) {
        for (int i = 0; i < column; ++i) {
            Element *element = new ColumnElement(i);
            m_index.append(element);
        }
    }
    void setLogicHeight(int height) override {
        m_height = height;
        for (auto *ele : m_index) {
            ele->setLogicHeight(m_height);
        }
    }
    void onNotify(EventContext &context, int type, int param1, int param2) override {
        if (context.outer) {
            context.outer->notify(type, param1, param2);
        }
    }

    void onRedraw(EventContext &context) override {
        Root::onRedraw(context);
    }
};
class TableElement : public Container {
public:
    TableElement(int line, int column) {
        for (int i = 0; i < line; ++i) {
            Element *element = new RowElement(column);
            append(element);
        }
    }
    int getLogicHeight(EventContext &context) override {
        return Container::getLogicHeight(context) + 1;
    }
    void replace(int line, int column, Element *element) {
        auto *row = (RowElement *) m_index.at(line);
        auto *old = row->m_index.at(column);
        row->m_index.at(column) = element;
        delete old;
    }
    void setColumnWidth(int column, int width) {
        for (auto & i : m_index) {
            if (i->hasChild()) {
                auto col = i->children()->at(column);
                col->setLogicWidth(width);
            }
        }
    }
    void onNotify(EventContext &context, int type, int p1, int p2) override {
        EventContext row = context.enter();
        int width = 0;
        while (row.has()) {
            EventContext column = row.enter(p2);
            int cur_width = column.current()->getLogicWidth(column);
            if (cur_width > width)
                width = cur_width;
            row.next();
        }
        setColumnWidth(p2, width);
        row.getLayoutManager()->reflowEnter(row.start());
        context.redraw();
    }
    void onLeaveReflow(EventContext &context) override {
        Buffer<int> maxWidthBuffer;
        EventContext row = context.enter();
        while (row.has()) {
            EventContext col = row.enter();
            while (col.has()) {
                int width = col.width();
                if (width > maxWidthBuffer[col.index]) {
                    maxWidthBuffer[col.index] = width;
                }
                col.next();
            }
            row.next();
        }
        for (auto iter = maxWidthBuffer.iter(); iter.has(); iter.next()) {
            setColumnWidth(iter.index(), iter.current());
        }
        maxWidthBuffer.clear();
        row.start().reflowEnter();
    }

};

#endif //GEDITOR_TABLE_H
