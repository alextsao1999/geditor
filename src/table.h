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
#include <iostream>
#include "document.h"
#include "utils.h"
#define TAG_FOCUS _GT("Focus")
class PathUtil {
public:
    static float rad(float deg) {
        return (float) (deg * 3.1415926 / 180);
    }
    static GPath triangleDown(GScalar length, GScalar dx, GScalar dy) {
        GPath path;
        float v = length * cos(rad(45));
        path.moveTo(0, 0);
        path.lineTo(v * 2, 0);
        path.lineTo(v, v);
        path.close();
        path.offset(dx - v, dy);
        return path;
    }
    static GPath triangleRight(GScalar length, GScalar dx, GScalar dy) {
        GPath path;
        float v = length * cos(rad(45));
        path.moveTo(0, 0);
        path.lineTo(0, v * 2);
        path.lineTo(v, v);
        path.close();
        path.offset(dx, dy - v);
        return path;
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

};
class TextCaretService {
protected:
    Offset m_offset;
    Offset m_move;
    EventContext* m_context = nullptr;
    int m_index = 0; // 没有匹配到m_move 就直接设置index
public:
    TextCaretService(const Offset& offset, EventContext* context) : m_offset(offset), m_context(context) {
        m_index = m_context->getCaretManager()->data().index;
    }
    void setTextOffset(Offset offset) { m_offset = offset; }
    int& index() { return m_index; }
    void moveLeft() { m_index--; }
    void moveRight() { m_index++; }
    void moveToIndex(int index) { m_index = index; }
    void moveTo(Offset offset) { m_move = offset; }
    bool commit(GStyle& style, SkRect* bound = nullptr, int column = 0) {
        LineViewer viewer = m_context->getLineViewer(column);
        return commit(viewer.c_str(), viewer.size(), style, bound);
    }
    bool commit() {
        return commit(m_context->getStyle(StyleDeafaultFont));
    }
    bool commit(GString &string, int style) {
        return commit(string.c_str(), string.length() * sizeof(GChar), m_context->getStyle(style));
    }
    bool commit(const void* text, size_t bytelength, GStyle& paint, SkRect* bound = nullptr) {
        bool res; // 是否未越界
        int index = m_index;
        int length = paint.countText(text, bytelength);
        auto *widths = new SkScalar[length];
        auto *rects = new SkRect[length];
        paint.getTextWidths(text, bytelength, widths, rects);
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
            m_move = Offset(0, 0);
        }
        return res;
    }
    void show(Offset offset, int index) {
        CaretManager* m_caret = m_context->getCaretManager();
        m_caret->data().set(index, m_context->absOffset(offset));
        m_caret->set(offset.x, offset.y);
        m_caret->show();
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
    Display getDisplay() override { return DisplayBlock; }
    int getLogicHeight(EventContext &context) override { return 100; }
    int getLogicWidth(EventContext &context) override { return 200; }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        canvas->drawBitmap(bitmap, 0, 0);
        canvas.drawRect(canvas.bound(0.5, 0.5), StyleTableBorder);
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
    void draw(int sigma, int posx = 0, int posy = 0, SkColor border = 0, SkColor color = 0) {

        SkRect rect{};
        bitmap.getBounds(&rect);
        rect.inset(20.5, 20.5);

        SkCanvas canvas(bitmap);
        canvas.clear(SK_ColorWHITE);

        SkPaint paint;
        paint.setStyle(SkPaint::Style::kFill_Style);
        paint.setColor(color);
        paint.setAntiAlias(true);
        paint.setLooper(SkBlurDrawLooper::Create(color, sigma, posx, posy))->unref();
        canvas.drawRoundRect(rect, 4, 4, paint);

        paint.reset();
        paint.setColor(border);
        paint.setStyle(SkPaint::Style::kStroke_Style);
        paint.setStrokeWidth(1);
        paint.setAntiAlias(true);
        canvas.drawRoundRect(rect, 4, 4, paint);

    }
    EventContext *ctx{nullptr};
    void onMouseEnter(EventContext &context, int x, int y) override {
        ctx = context.copy();
        draw(11, 0, 0,
                SkColorSetRGB(161, 161, 161), SkColorSetRGB(222, 222, 222));
        context.redraw();
    }
    void onMouseLeave(int x, int y) override {
        draw(0, 0, 0,
             SkColorSetRGB(191, 191, 191), SkColorSetRGB(242, 242, 242));
        ctx->redraw();
        ctx->free();
    }
    void onLeftButtonUp(EventContext &context, int x, int y) override {
        draw(11, 0, 0,
             SkColorSetRGB(161, 161, 161), SkColorSetRGB(222, 222, 222));
        context.redraw();
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        draw(11, 0, 0,
             SkColorSetRGB(166, 166, 166), SkColorSetRGB(204, 204, 204));
        context.redraw();
        context.timer(100, 100);
    }

    bool direction = false;
    int counter = 0;
    int tranform = 0;
    bool onTimer(EventContext &context, int id) override {
        if (!context.visible()) {
            return true;
        }
        if (counter >= 20 || counter < 0) {
            direction = !direction;
            tranform++;
        }
        counter = direction ? counter - 1 : counter + 1;
        draw(counter, 0, 0,
             SkColorSetRGB(166, 166, 166), SkColorSetRGB(204, 204, 204 + counter));
        context.redraw();
        return true;
    }
};
class MoveElement : public RelativeElement {
public:
    Display getDisplay() override { return DisplayBlock; }
    int getLogicHeight(EventContext &context) override { return 100; }
    int getLogicWidth(EventContext &context) override { return 200; }
    bool is_moving = false;
    Offset m_click;
    void onLeftButtonUp(EventContext &context, int x, int y) override {
        is_moving = false;
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        is_moving = true;
        m_click = context.relative(x, y);
    }
    void onMouseMove(EventContext &context, int x, int y) override {
        if (is_moving) {
            Offset offset = context.relative(x, y) - m_click;
            Offset logic = getLogicOffset() + offset;
            setLogicOffset(logic);
            context.reflow();
            context.redraw();
        }
    }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        canvas.drawRect(context.logicRect(), StyleTableBorder);
    }
};
class LineElement : public RelativeElement {
public:
    LineElement() = default;
    int getLogicHeight(EventContext &context) override { return 25; }
    //int getLogicWidth(EventContext &context) override { return 25; }
    Display getDisplay() override { return DisplayLine; }
    Tag getTag(EventContext &context) override { return {_GT("LineElement Focus")}; }
    Element *copy() override { return new LineElement(); }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        SkPaint border;
        border.setStyle(SkPaint::Style::kStroke_Style);
        border.setColor(SK_ColorLTGRAY);
        canvas->drawRect(canvas.bound(0.5, 0.5), border);
        LineViewer viewer = context.getLineViewer();
        canvas.translate(0, context.getStyle(StyleDeafaultFont).getTextSize());
        canvas.drawText(viewer.c_str(), viewer.size(), 4, 2, StyleDeafaultFont);
        drawLight(context);
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.pos().setOffset(context.absolute(x, y));
        context.focus();
    }
    void onMouseMove(EventContext &context, int x, int y) override {
        if (context.selecting()) {
            context.pos().setOffset(context.absolute(x, y));
            context.focus();
        }
    }
    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        TextCaretService service(Offset(4, 5), &context);
        if (code == VK_LEFT) {
            service.moveLeft();
            if (!service.commit()) {
                caret->data().setIndex(-1);
                caret->findPrev(TAG_FOCUS);
            }
        }
        if (code == VK_RIGHT) {
            service.moveRight();
            if (!service.commit()) {
                caret->data().setIndex(0);
                caret->findNext(TAG_FOCUS);
            }
        }
        if (code == VK_UP) {
            caret->findPrev(TAG_FOCUS);
        }
        if (code == VK_DOWN) {
            caret->findNext(TAG_FOCUS);
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
/*
                        service.moveToIndex(-1);
                        service.commit();
                        context.combine(); // 因为combine要delete本对象 之后paint就不存在了 所以移动要在之前调用
                        context.reflow();
                        context.redraw();
*/
                    } else {
                        context.pos().setIndex(-1);
                        caret->findPrev(TAG_FOCUS);
                    }
                    return;
                }
                break;
            case VK_RETURN:
                //context.copyLine();
                context.reflow();
                if (caret->next()) {
/*
                    int idx = service.index();
                    context.getLineViewer().append(line.c_str() + idx, line.length() - idx);
                    line.erase(idx, line.length() - idx);
                    service.moveToIndex(0);
*/
                }
                break;
            default:
                line.insert(service.index(), ch);
                context.push(CommandType::Add, CommandData(service.index(), ch));
                service.moveRight();
                break;
        }
        service.commit();
        context.redraw();
    }
    void onRightButtonUp(EventContext &context, int x, int y) override {
        context.timer(500, 0, 4);
    }
    void onFocus(EventContext &context) override {
        auto caret = context.getCaretManager();
        TextCaretService service(Offset(4, 6), &context);
        service.moveTo(context.relOffset(caret->data().getOffset()));
        service.commit();
        // 选中
        //SkColorFilter::CreateModeFilter(SK_ColorLTGRAY, SkXfermode::Mode::kSrcOut_Mode)
        // 设置文字
        //SkColorFilter::CreateModeFilter(SK_ColorLTGRAY, SkXfermode::Mode::kSrcIn_Mode);
        // 设置背景
        //SkColorFilter::CreateModeFilter(SK_ColorLTGRAY, SkXfermode::Mode::kColorDodge_Mode);
/*
        style.setColorFilter(SkColorFilter::CreateModeFilter(
                        SkColorSetARGB(255, 255, 250, 227), SkXfermode::Mode::kDarken_Mode));
*/

    }
    bool show = false;
    void drawLight(EventContext &context) {
        if (show) {
            Canvas canvas = context.getCanvas();
            SkPaint border;
            border.setStyle(SkPaint::Style::kStroke_Style);
            border.setColor(SK_ColorRED);
            canvas->drawRect(canvas.bound(0.5, 0.5), border);
            SkPaint font;
            font.setTextAlign(SkPaint::Align::kRight_Align);
            char str[255];
            sprintf(str, "%s : %d ", context.path().c_str(), context.getLineCounter().line);
            canvas->drawText(str, strlen(str), context.width(), 15, font);
        }
    }
    bool onTimer(EventContext &context, int id) override {
        show = !show;
        context.redraw();
        return true;
    }
};
class SyntaxLineElement : public LineElement {
    Element *copy() override { return new SyntaxLineElement(); }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        SkPaint border;
        if (context.selected()) {
            border.setStyle(SkPaint::Style::kStrokeAndFill_Style);
        } else {
            border.setStyle(SkPaint::Style::kStroke_Style);
        }
        border.setColor(SK_ColorLTGRAY);
        //canvas->drawRect(canvas.bound(0.5, 0.5), border);
        auto *lexer = context.getLexer();
        Offset offset(4, context.height() - 4);
        while (lexer->has()) {
            Token token = lexer->next();
            if (token == TokenIdentifier && token.style != StyleKeywordFont) {
                if (lexer->peek() == _GT("(") || (lexer->peek() == TokenSpace && lexer->peek(2) == _GT("("))) {
                    token.style = StyleFunctionFont;
                }
            }
            GStyle &token_style = context.getStyle(token.style);
            canvas->drawText(token.c_str(), token.size(), offset.x, offset.y, token_style.paint());
            offset.x += token_style.measureText(token.c_str(), token.size());
        }

        drawLight(context);

    }
};
class TextElement : public RelativeElement {
private:
    int m_min = 50;
public:
    GString m_data;
    int m_width = 0;
    int m_height = 25;
    explicit TextElement() = default;
    Tag getTag(EventContext &context) override { return {_GT("TextElement Focus")}; }
    int getMinWidth(EventContext &context) override {
        int width = (int) context.getStyle(StyleTableFont)
                .measureText(m_data.c_str(), m_data.length() * sizeof(GChar)) + 15;
        width = width > m_min ? width : m_min;
        return width;
    }
    int getLogicHeight(EventContext &context) override { return m_height; }
    int getLogicWidth(EventContext &context) override {
        int width = getMinWidth(context);
        return width > m_width ? width : m_width;
    }
    void setLogicWidth(EventContext &context, int width) override { m_width = width; }
    void setLogicHeight(EventContext &context, int height) override { m_height = height; }
    Display getDisplay() override { return DisplayInline; }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        if (context.outer && context.outer->selected()) {
            canvas.drawRect(canvas.bound(0.5, 0.5), StyleTableBorderSelected);
        } else {
            canvas.drawRect(canvas.bound(0.5, 0.5), StyleTableBorder);
        }
        canvas.translate(0, context.getStyle(StyleTableFont).getTextSize());
        canvas.drawText(m_data.c_str(), m_data.length() * sizeof(GChar), 4, 2, StyleTableFont);
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.pos().setOffset(context.absolute(x, y));
        context.focus();
    }
    void onMouseMove(EventContext &context, int x, int y) override {
        if (context.selecting()) {
            context.pos().setOffset(context.absolute(x, y));
            context.focus();
        }
    }
    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        TextCaretService service(Offset(4, 4), &context);
        if (code == VK_LEFT) {
            service.moveLeft();
            if (!service.commit(m_data, StyleTableFont)) {
                caret->data().setIndex(-1);
                caret->findPrev(TAG_FOCUS);
            }
        }
        if (code == VK_RIGHT) {
            service.moveRight();
            if (!service.commit(m_data, StyleTableFont)) {
                caret->data().setIndex(0);
                caret->findNext(TAG_FOCUS);
            }
        }
        if (code == VK_DOWN) {
            //service.commit(m_data, StyleTableFont);
        }
        if (code == VK_UP) {
            //service.commit(m_data, StyleTableFont);
        }

    };
    void onInputChar(EventContext &context, int ch) override {
        TextCaretService service(Offset(4, 4), &context);
        switch (ch) {
            case VK_BACK:
                if (service.index() > 0) {
                    m_data.erase(service.index() - 1, 1);
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
        service.commit(m_data, StyleTableFont);
        if (context.outer) {
            context.outer->notify(Update, 0, 0);
        }
        context.redraw();
    }
    void onFocus(EventContext &context) override {
        TextCaretService service(Offset(4, 4), &context);
        service.moveTo(context.relOffset(context.pos().getOffset()));
        service.commit(m_data, StyleTableFont);
    }

};
class RowElement : public Container<DisplayRow> {
public:
    explicit RowElement(int column) {
        for (int i = 0; i < column; ++i) {
            append(new TextElement());
        }
    }
    void onNotify(EventContext &context, int type, int param, int other) override {
        if (context.outer) {
            context.outer->notify(type, param, other);
        }
    }
    TextElement *getCol(int col) { return (TextElement *) get(col); }
};
class TableElement : public Container<DisplayTable> {
public:
    int m_delta = 0;
    TableElement(int line, int column) {
        for (int i = 0; i < line; ++i) {
            append(new RowElement(column));
        }
    }
    void replace(int line, int column, Element *element) {
        auto *row = (Container *) get(line);
        row->replace(column, element)->free();
    }
    template <typename Type = TextElement>
    Type *getItem(int row, int col) { return (Type *) ((RowElement *) get(row))->get(col); }
    void onNotify(EventContext &context, int type, int param, int other) override {
        if (context.outer && context.outer->display() == DisplayRow) {
            context.outer->notify(type, param, other);
        } else {
            context.relayout();
            context.reflow();
        }
    }
    void setLogicWidth(EventContext &context, int width) override {
        m_delta = width - m_width;
        for_context(row, context) {
            EventContext end = row.enter(-1);
            end.setLogicWidth(end.logicWidth() + m_delta);
        }
    }
    int getLogicWidth(EventContext &context) override {
        return m_width + m_delta;
    }
    int getMinWidth(EventContext &context) override {
        return m_width;
    }
    void onEnterReflow(EventContext &context, Offset &offset) override {
        offset.x += 4;
        offset.y += 5;
    }

};
class CodeBlockElement : public Container<> {
public:
    CodeBlockElement() {
        append(new SyntaxLineElement());
        append(new SyntaxLineElement());
    }

    Tag getTag(EventContext &context) override {
        return _GT("CodeBlock");
    }
    int getWidth(EventContext &context) override {
        return Element::getWidth(context);
    }
    void onEnterReflow(EventContext &context, Offset &offset) override {
        offset.x += 25;
    }
    void onRedraw(EventContext &context) override {
        Container::onRedraw(context);
        return;
        SkPaint paint;
        paint.setAntiAlias(true);
        SkPoint points[2] = {{0.0f, 0.0f}, {3.0f, 3.0f}};
        SkColor colors[2] = {SkColorSetRGB(66,133,244), SkColorSetRGB(15,157,88)};
        paint.setShader(
                SkGradientShader::CreateLinear(
                        points, colors, NULL, 2, SkShader::TileMode::kClamp_TileMode, 0, NULL))
                ->unref();
        //paint.setColor(SK_ColorBLACK);
        for_context(ctx, context) {
            Canvas canvas = ctx.getCanvas();
            GPath sym = PathUtil::star(5, 5, 2);
            sym.offset(-8, 10);
            canvas->drawPath(sym, paint);
//            canvas->drawCircle(-5, 15, 5.5, paint);

        }
    }

};
class SwitchElement : public Container<> {
public:
    SwitchElement() {
        append(new CodeBlockElement());
        append(new CodeBlockElement());
    }
    Tag getTag(EventContext &context) override { return {_GT("Switch CodeBlock")}; }
    CodeBlockElement *addBlock() {
        return (CodeBlockElement *) append(new CodeBlockElement());
    }
private:
    int getWidth(EventContext &context) override {
        return Element::getWidth(context);
    }
    void onRedraw(EventContext &context) override {
        Container::onRedraw(context);
        SkPaint paint;
        GScalar inter[2] = {3, 2};
        paint.setPathEffect(SkDashPathEffect::Create(inter, 2, 25))->unref();
        paint.setColor(SK_ColorBLACK);
        paint.setAlpha(180);
        Offset offset = context.offset();
        Offset runStart;
        for_context(ctx, context) {
            Canvas canvas = ctx.getCanvas();
            if (ctx.isHead()) {
                runStart = ctx.enter(-1).offset() - offset;
            }
            if (!ctx.isTail()) {
                int lineTop = ctx.isHead() ? 15 : 20;
                if (ctx.isHead() && (
                        context.parent().tag() == _GT("CodeBlock") ||
                        context.nearby(-1).tag().contain(_GT("CodeBlock")))) {
                    lineTop = 20;
                }
                canvas->drawLine(-15, lineTop, 0, lineTop, paint);

                int lineBottom = ctx.height() + 15;
                canvas->drawLine(-15, lineTop, -15, lineBottom, paint); // 竖线
                int underlineRight;
                if (ctx.nearby(1).enter().tag().contain(_GT("CodeBlock"))) {
                    underlineRight = 25;
                } else {
                    underlineRight = 0;
                }
                canvas->drawLine(-15, lineBottom, underlineRight, lineBottom, paint); // 下边线
                GPath path = PathUtil::triangleRight(6, underlineRight - 2, lineBottom);
                canvas->drawPath(path, paint);// 下边线三角形
                canvas->drawLine(-6, ctx.height() - 10, 0, ctx.height() - 10, paint); // 逃逸线横线
            }
        }
        Canvas canvas = context.getCanvas();
        if (context.nearby(1).tag().contain(_GT("CodeBlock"))) {
            int runawayBottom = context.height() + 15;
            // 逃逸线竖线
            canvas->drawLine(18, runStart.y + 15, 18, runawayBottom, paint);
            // 逃逸线 下边右横线
            canvas->drawLine(18, runawayBottom, 24, runawayBottom, paint);
            GPath path = PathUtil::triangleRight(6, 23, runawayBottom); // 右三角
            canvas->drawPath(path, paint);
        } else {
            // 逃逸线竖线
            canvas->drawLine(18, runStart.y + 15, 18, context.height(), paint);
            // 逃逸线向下
            GPath path = PathUtil::triangleDown(6, 18, context.height() - 2);
            // 有可能后面还有流程语句 需要连接
            canvas->drawPath(path, paint);
        }
    }
};
class SingleBlockElement : public CodeBlockElement {
public:
    Tag getTag(EventContext &context) override { return _GT("Single CodeBlock"); }
    void onRedraw(EventContext &context) override {
        CodeBlockElement::onRedraw(context);
        Canvas canvas = context.getCanvas();
        SkPaint paint;
        GScalar inter[2] = {3, 2};
        paint.setPathEffect(SkDashPathEffect::Create(inter, 2, 25))->unref();
        paint.setColor(SK_ColorBLACK);
        paint.setAlpha(180);

        int lineTop = 15;
        if (context.parent().tag() == _GT("CodeBlock") ||
            context.nearby(-1).tag().contain(_GT("CodeBlock"))) {
            lineTop = 20;
        }
        canvas->drawLine(-15, lineTop, 0, lineTop, paint);
        if (context.nearby(1).tag().contain(_GT("CodeBlock"))) {
            int lineBottom = context.height() + 15;
            canvas->drawLine(-15, lineTop, -15, lineBottom, paint); // 竖线

            canvas->drawLine(-15, lineBottom, 0, lineBottom, paint); // 下边线
            GPath path = PathUtil::triangleRight(6, 0, lineBottom);
            canvas->drawPath(path, paint);
        } else {
            int lineBottom = context.height() - 5;
            canvas->drawLine(-15, lineTop, -15, lineBottom, paint); // 竖线
            GPath path = PathUtil::triangleDown(6, -15, lineBottom);
            canvas->drawPath(path, paint);
        }
    }
};
class SubElement : public Container<> {
public:
    SubElement() {
        auto *table = new TableElement(2, 4);
        table->getItem(0, 0)->m_data.append(_GT("function"));
        table->getItem(0, 1)->m_data.append(_GT("function"));
        table->replace(1, 3, new TableElement(2, 2));
        //table->replace(0, 3, new ButtonElement());
        append(table);
        append(new SyntaxLineElement());

        auto *control = new SwitchElement();
        control->addBlock()->replace(0, new SwitchElement());
        append(control);
        append(new SwitchElement());
        append(new SingleBlockElement());
        append(new SyntaxLineElement());

    }
    Tag getTag(EventContext &context) override { return {_GT("SubElement")}; }
private:
    int getWidth(EventContext &context) override {
        return Element::getWidth(context);
    }

};
#endif //GEDITOR_TABLE_H
