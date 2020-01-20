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
#define context_on_outer(ctx, method, ...) if ((ctx).outer) context_on(*((ctx).outer), method, ##__VA_ARGS__);
class PathUtil {
public:
    inline static float rad(float deg) { return deg * 3.1415926 / 180; }
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
    bool commit(int style = StyleDeafaultFont) {
        LineViewer viewer = m_context->getLineViewer();
        return commit(viewer.c_str(), viewer.size(), m_context->getStyle(style), nullptr);
    }
    bool commit(GString &string, int style = StyleDeafaultFont) {
        return commit(string.c_str(), string.length() * sizeof(GChar), m_context->getStyle(style));
    }
    bool commit(const void* text, size_t bytelength, GStyle& paint, SkRect* bound = nullptr) {
        bool res; // 是否未越界
        int index = m_index;
        int length = paint.countText(text, bytelength);
        Buffer<SkScalar> widths(length);
        Buffer<SkRect> rects(length);
        paint.getTextWidths(text, bytelength, widths.data, rects.data);
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
        if (res) {
            m_move = Offset(0, 0);
        }
        return res;
    }
    void breakText(const void *text, size_t bytelength, GStyle &paint, Buffer<SkRect> &rects) {
        int length = paint.countText(text, bytelength);
        Buffer<SkScalar> widths(length);
        rects.reset(length);
        paint.getTextWidths(text, bytelength, widths.data, rects.data);
        SkScalar size = paint.getTextSize();
        int width = 0;
        for (int i = 0; i < length; ++i) {
            rects[i].offset(m_offset.x + width, size + (SkScalar) m_offset.y);
            width += widths[i];
        }
    }
    void breakText(Buffer<SkRect> &rects, int style = StyleDeafaultFont) {
        LineViewer viewer = m_context->getLineViewer(0);
        breakText(viewer.c_str(), viewer.size(), m_context->getStyle(style), rects);
    }
    CaretPos getIndexPos(Buffer<SkRect> &rects, int index) {
        return {index, m_context->absOffset(Offset(rects[index].fLeft, m_offset.y))};
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
    Tag getTag(EventContext &context) override {
        Tag tag = {_GT("Line Focus")};
        tag.append(_GT("(")).append(context.getCounter().line).append(_GT(")"));
        return tag;
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.pos().setOffset(context.absolute(x, y));
        context.focus();
        context.redraw();
    }
    void onLeftDoubleClick(EventContext &context, int x, int y) override {
        TextCaretService service(Offset(4, 6), &context);
        auto line = context.getLineViewer();
        Buffer<SkRect> rects;
        service.breakText(rects);
        int length = line.length();
        int startIndex = service.index();
        int endIndex = service.index();
        while (startIndex - 1 >= 0 && IsCodeChar(line.charAt(startIndex - 1))) {
            startIndex--;
        }
        while (endIndex < length && IsCodeChar(line.charAt(endIndex))) {
            endIndex++;
        }
        CaretPos start = service.getIndexPos(rects, startIndex);
        CaretPos end = service.getIndexPos(rects, endIndex);
        context.getDocContext()->setSelectPos(start, end);
        context.setPos(end);
        context.focus();
        context.redraw();
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
    void onSelectionDelete(EventContext &context, SelectionState state) override {
        auto line = context.getLineViewer();
        auto *dctx = context.getDocContext();
        if (state == SelectionSelf) {
            CaretPos start = context.getDocContext()->m_selectStartPos;
            CaretPos end = context.getDocContext()->m_selectEndPos;
            if (start.index > end.index) {
                std::swap(start, end);
            }
            int length = end.index - start.index;
            context.push(CommandType::DeleteString,
                         CommandData(start.index, new GString(line.c_str() + start.index, length)));
            line.remove(start.index, length);
            dctx->clearSelect();
            context.setPos(start);
        }

        if (state == SelectionStart) {
            CaretPos start = context.getDocContext()->m_selectStartPos;
            int length = line.length() - start.index;
            context.push(CommandType::DeleteString,
                         CommandData(start.index, new GString(line.c_str() + start.index, length)));
            line.remove(start.index, length);
            context.setPos(start);
            context.focus();
        }

        if (state == SelectionInside || state == SelectionRow) {
            context.push(CommandType::DeleteString,
                         CommandData(0, new GString(line.c_str(), line.length())));
            erase(context);
        }

        if (state == SelectionEnd) {

            CaretPos end = context.getDocContext()->m_selectEndPos;
            context.push(CommandType::DeleteString,
                         CommandData(0, new GString(line.c_str(), end.index)));
            line.remove(0, end.index);

            EventContext ctx = context.nearby(-1);
            if (ctx.tag().contain(_GT("Line"))) {
                ctx.push(CommandType::Combine, CommandData(ctx.getLineViewer().length()));
                ctx.combineLine();
                erase(context);
            } else {
                context.pos().setIndex(0);
                context.focus();
            }

            dctx->clearSelect();
        }

    }
    void onInputChar(EventContext &context, SelectionState state, int ch) override {
        auto* caret = context.getCaretManager();
        TextCaretService service(Offset(4, 6), &context);
        auto line = context.getLineViewer();
        switch (ch) {
            case VK_BACK:
                if (state == SelectionNone) {
                    if (service.index() > 0) {
                        context.push(CommandType::DeleteChar,
                                     CommandData(
                                             service.index() - 1,
                                             line.charAt(service.index() - 1)));
                        line.remove(service.index() - 1);
                        service.moveLeft();
                    } else {
                        EventContext prev = context.nearby(-1);
                        if (prev.tag().contain(_GT("Line"))) {
                            auto prevLine = prev.getLineViewer();
                            int length = prevLine.length();
                            context.pos().setIndex(length);
                            prev.push(CommandType::Combine, CommandData(length));
                            prevLine.append(line.c_str());
                            erase(context);
                            prev.reflow();
                            prev.redraw();
                            prev.focus();
                            return;
                        } else {
                            caret->data().setIndex(-1);
                            caret->findPrev(TAG_FOCUS);
                            return;
                        }
                    }
                }
                break;
            case VK_RETURN: {
                insert(context);
                int idx = service.index();
                context.push(CommandType::Break, CommandData(idx));
                auto next = context.getLineViewer(1);
                next.append(line.c_str() + idx, line.length() - idx);
                line.remove(idx, line.length() - idx);
                context.reflow();
                context.redraw();
                context.pos().setIndex(0);
                caret->data().setIndex(0);
                caret->next();
                return;
            }
            default:
                context.push(CommandType::AddChar, CommandData(service.index(), ch));
                line.insert(service.index(), ch);
                service.moveRight();
                break;
        }
        service.commit();
        context.redraw();
    }
    void onFocus(EventContext &context) override {
        context_on_outer(context, Focus);
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
    void onBlur(EventContext &context, EventContext *focus, bool force) override {
        context_on_outer(context, Blur, focus, force);
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
            sprintf(str, "%s : %d ", context.path().c_str(), context.getCounter().line);
            canvas->drawText(str, strlen(str), context.width(), 15, font);
        }
    }
    bool onTimer(EventContext &context, int id) override {
        show = !show;
        context.redraw();
        return true;
    }
    virtual void insert(EventContext &context) {
        Element *element = copy();
        context.push(CommandType::AddElement, CommandData(element));
        context.insertLine(1);
        element->setPrev(this);
        element->setNext(m_next);
        if (m_next) {
            m_next->setPrev(element);
        } else {
            if (context.outer)
                context.outer->current()->setTail(element);
        }
        m_next = element;
    }
    virtual void erase(EventContext &context) {
        context.deleteLine();
        context.push(CommandType::DeleteElement, CommandData(this));
        context.prevLine(getLineNumber());
        separate(context, m_next, m_prev);
    }
    virtual Element *copy() { return new LineElement(); }
    void onUndo(Command command) override {
        bool undo = false;
        command.context->setPos(command.pos); // 因为替换之前都是LineElement 所以直接focus就行
        command.context->focus(true, true);
        auto line = command.context->getLineViewer();
        if (command.type == CommandType::Break) {
            auto next = command.context->getLineViewer(1);
            line.append(next.c_str());
            next.clear();
            undo = true;
        }
        if (command.type == CommandType::Combine) {
            command.context->breakLine(0, command.data.value);
        }
        if (command.type == CommandType::AddChar) {
            line.remove(command.data.input.pos);
        }
        if (command.type == CommandType::DeleteChar) {
            line.insert(command.data.input.pos, command.data.input.ch);
            command.context->pos().setIndex(command.pos.index + 1);
            command.context->focus(false, true);
        }
        if (command.type == CommandType::AddElement) {
            command.context->deleteLine(1);
        }
        if (command.type == CommandType::DeleteElement) {
            command.context->insertLine();
            undo = true;
        }
        if (command.type == CommandType::DeleteString) {
            line.insert(command.data.string.pos, command.data.string.string->c_str());
            delete command.data.string.string;
        }
        Element::onUndo(command);
        if (undo) {
            command.context->doc->undo();
        }
    }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        SkPaint border;
        border.setStyle(SkPaint::Style::kStroke_Style);
        border.setColor(SK_ColorLTGRAY);
        canvas->drawRect(canvas.bound(0.5, 0.5), border);
        LineViewer viewer = context.getLineViewer();
        canvas.translate(0, context.getStyle(StyleDeafaultFont).getTextSize());
        canvas.drawText(viewer.c_str(), viewer.size(), 4, 2, StyleDeafaultFont);
    }
    void onDraw(EventContext &context, Drawable canvas) override {
        SkPaint border;
        border.setStyle(SkPaint::Style::kStroke_Style);
        border.setColor(SK_ColorLTGRAY);
        canvas->drawRect(context.bound(0.5), border);
        LineViewer viewer = context.getLineViewer();
        canvas->translate(0, context.getStyle(StyleDeafaultFont).getTextSize());
        canvas->drawText(viewer.c_str(), viewer.size(), 4, 4, context.getStyle(StyleDeafaultFont).paint());
    }
};
class SyntaxLineElement : public LineElement {
public:
    Element *copy() override { return new SyntaxLineElement(); }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        SkPaint border;
        if (context.isSelectedRow()) {
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
    void onDraw(EventContext &context, Drawable canvas) override {
        SkPaint border;
        if (context.isSelectedRow()) {
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
class AutoLineElement : public SyntaxLineElement {
public:
    Element *copy() override { return new AutoLineElement(); }
    void onInputChar(EventContext &context, SelectionState state, int ch) override;

    void onRedraw(EventContext &context) override {
        SelectionState state = context.getSelectionState();
        if (state == SelectionSelf) {
            Offset start = context.relOffset(context.getDocContext()->m_selectStart);
            Offset end = context.relOffset(context.getDocContext()->m_selectEnd);
            DrawSlection(context, start, end);
        }
        if (state == SelectionStart) {
            TextCaretService text(Offset(4, 6), &context);
            Buffer<SkRect> rects;
            text.breakText(rects);
            Offset start = context.relOffset(context.getDocContext()->m_selectStart);
            if (rects.count) {
                DrawSlection(context, start, Offset(rects.back().right(), rects.back().top()));
            }
        }
        if (state == SelectionEnd) {
            Offset end = context.relOffset(context.getDocContext()->m_selectEnd);
            DrawSlection(context, Offset(4, 6), end);
        }
        if (state == SelectionInside || state == SelectionRow) {
            TextCaretService text(Offset(4, 6), &context);
            Buffer<SkRect> rects;
            text.breakText(rects);
            if (rects.count) {
                DrawSlection(context, Offset(4, 6), Offset(rects.back().right(), rects.back().top()));
            } else {
                DrawSlection(context, Offset(4, 6), Offset(10, 6));
            }
        }
        SyntaxLineElement::onRedraw(context);
    }

    static void DrawSlection(EventContext &context, Offset start, Offset end) {
        Canvas canvas = context.getCanvas();
        GRect rect;
        rect.set({(GScalar) start.x, (GScalar) start.y}, {(GScalar) end.x, (GScalar) end.y});
        rect.fTop -= 2;
        rect.fBottom = rect.fTop + context.getStyle(StyleDeafaultFont)->getTextSize() + 8;
        SkPaint paint;
        paint.setColor(SkColorSetRGB(204, 226, 254));
        paint.setAlpha(255);
        paint.setAntiAlias(true);
        canvas->drawRoundRect(rect, 4, 4, paint);
        paint.setColor(SkColorSetRGB(150, 200, 255));
        paint.setStyle(SkPaint::kStroke_Style);
        rect.inset(0.5f, 0.5f);
        canvas->drawRoundRect(rect, 4, 4, paint);
    }
};
class TextElement : public RelativeElement {
private:
public:
    int m_min = 50;
    GString m_data;
    int m_width = 0;
    int m_height = 22;
    explicit TextElement() = default;
    Tag getTag(EventContext &context) override {
        Tag tag = {_GT("Text Focus")};
        tag.append(_GT("("));
        tag.append(context.index);
        if (context.outer) {
            tag.append(_GT(","));
            tag.append(context.outer->index);
        }
        tag.append(_GT(")"));
        return tag;
    }
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
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.pos().setOffset(context.absolute(x, y));
        context.focus();
        context.redraw();
    }
    void onMouseMove(EventContext &context, int x, int y) override {
        if (context.selecting()) {
            context.pos().setOffset(context.absolute(x, y));
            context.focus();
        }
    }
    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        TextCaretService service(Offset(6, 2), &context);
        if (code == VK_LEFT) {
            if (context.outer->tag().contain(_GT("Uneditable"))) {
                caret->data().setIndex(-1);
                caret->findPrev(TAG_FOCUS);
                return;
            }
            service.moveLeft();
            if (!service.commit(m_data, StyleTableFont)) {
                caret->data().setIndex(-1);
                caret->findPrev(TAG_FOCUS);
            }
        }
        if (code == VK_RIGHT) {
            if (context.outer->tag().contain(_GT("Uneditable"))) {
                caret->data().setIndex(0);
                caret->findNext(TAG_FOCUS);
                return;
            }
            service.moveRight();
            if (!service.commit(m_data, StyleTableFont)) {
                caret->data().setIndex(0);
                caret->findNext(TAG_FOCUS);
            }
        }
        if (code == VK_DOWN) {
            if (context.outer->isTail()) { // 寻找表格外的下一个元素 (判断是否在尾行)
                EventContext *next = context.outer->findNext(TAG_FOCUS);
                if (next) {
                    next->focus(false);
                }
            } else {
                context.outer->nearby(1).focus();
            }
            //service.commit(m_data, StyleTableFont);
        }
        if (code == VK_UP) {
            if (context.outer->isHead()) { // 判断是否为行首
                EventContext *real = context.getOuter(3);
                if (real && real->tag().contain(_GT("Row"))) {
                    real->nearby(-1).focus();
                    return;
                }
                EventContext *prev = context.outer->findPrev(TAG_FOCUS);
                if (prev) {
                    prev->focus(false);
                }
            } else {
                EventContext ctx = context.outer->nearby(-1);
                ctx.focus();

            }

            //service.commit(m_data, StyleTableFont);
        }

    };
    void onInputChar(EventContext &context, SelectionState state, int ch) override {
        if (context.outer->tag().contain(_GT("Uneditable"))) {
            return;
        }
        TextCaretService service(Offset(6, 2), &context);
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
        if (context.outer->tag().contain(_GT("Uneditable"))) {
            context.setPos(CaretPos(0, context.absOffset(Offset(6, 2))));
            context.getCaretManager()->set(6, 2);
            return;
        }

        TextCaretService service(Offset(6, 2), &context);
        service.moveTo(context.relOffset(context.pos().getOffset()));
        service.commit(m_data, StyleTableFont);
    }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        canvas.drawRect(canvas.bound(0.5, 0.5), StyleTableBorder);
        if (context.isSelectedSelf()) {
            Offset start = context.relOffset(context.getDocContext()->m_selectStart);
            Offset end = context.relOffset(context.getDocContext()->m_selectEnd);
            DrawSlection(context, start, end);
        }
        canvas.translate(0, context.getStyle(StyleTableFont).getTextSize());
        canvas.drawText(m_data.c_str(), m_data.length() * sizeof(GChar), 6, 3, StyleTableFont);
    }
    void onDraw(EventContext &context, Drawable canvas) override {
        if (context.outer && context.outer->isSelectedRow()) {
            canvas->drawRect(context.bound(0.5, 0.5), context.getStyle(StyleTableBorderSelected).paint());
        } else {
            canvas->drawRect(context.bound(0.5, 0.5), context.getStyle(StyleTableBorder).paint());
        }
        canvas->translate(0, context.getStyle(StyleTableFont).getTextSize());
        canvas->drawText(m_data.c_str(), m_data.length() * sizeof(GChar), 4, 2, context.getStyle(StyleTableFont).paint());
    }
    static void DrawSlection(EventContext &context, Offset start, Offset end) {
        Canvas canvas = context.getCanvas();
        GRect rect;
        rect.set({(GScalar) start.x, (GScalar) start.y}, {(GScalar) end.x, (GScalar) end.y});
        rect.fBottom = rect.fTop + context.getStyle(StyleDeafaultFont)->getTextSize() + 5;
        SkPaint paint;
        paint.setColor(SkColorSetRGB(204, 226, 254));
        paint.setAlpha(255);
        paint.setAntiAlias(true);
        canvas->drawRoundRect(rect, 4, 4, paint);
        paint.setColor(SkColorSetRGB(150, 200, 255));
        paint.setStyle(SkPaint::kStroke_Style);
        rect.inset(0.5f, 0.5f);
        canvas->drawRoundRect(rect, 4, 4, paint);
    }

};
class RowElement : public Container<DisplayRow> {
public:
    GColor m_color;
    explicit RowElement(int column, GColor color = SK_ColorWHITE) : m_color(color) {
        for (int i = 0; i < column; ++i) {
            Container::append(new TextElement());
        }
    }
    Tag getTag(EventContext &context) override { return {_GT("Row Focus")}; }
    TextElement *getColumn(int index) { return (TextElement *) get(index); }
    void setColor(GColor color) {
        m_color = color;
    }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        SkPaint paint;
        paint.setColor(m_color);
        canvas->drawRect(canvas.bound(0, 0), paint);
        if (context.isSelectedRow()) {
            if (context.isSelectedSelf()) {
                if (context.selectedCount() > 1) {
                    canvas.drawRect(canvas.bound(0.5, 0.5), StyleTableBorderSelected);
                }
            } else {
                canvas.drawRect(canvas.bound(0.5, 0.5), StyleTableBorderSelected);
            }
        }
        Root::onRedraw(context);
    }
    void onFocus(EventContext &context) override {
        CaretManager *caret = context.getCaretManager();
        Offset offset = caret->data().getOffset();
        for_context(ctx, context) {
            GRect rect = ctx.rect();
            if ((float) offset.x > rect.left() && (float) offset.x < rect.right()) {
                ctx.focus();
                return;
            }
        }
        context.enter().focus();
    }
};
class TableElement : public Container<DisplayTable> {
public:
    int m_delta = 0;
    int m_top;
    TableElement(int line, int column, int top = 5) {
        m_top = top;
        for (int i = 0; i < line; ++i) {
            Container::append(new RowElement(column));
        }
    }
    Tag getTag(EventContext &context) override { return {_GT("Table Focus")}; }
    RowElement *addRow(int column) {
        auto *row = new RowElement(column);
        append(row);
        return row;
    }
    RowElement *getRow(int line) { return (RowElement *) get(line); }
    void replace(int line, int column, Element *element) {
        auto *row = (Container *) get(line);
        row->replace(column, element)->free();
    }
    template <typename Type = TextElement>
    Type *getItem(int row, int col) { return (Type *) ((RowElement *) get(row))->get(col); }
    void onNotify(EventContext &context, int type, NotifyValue param, NotifyValue other) override {
        if (context.outer && context.outer->display() == DisplayRow) {
            context.outer->notify(type, param, other);
        } else {
            context.update();
        }
    }
    void setLogicWidth(EventContext &context, int width) override {
        m_delta = width - m_width;
        for_context(row, context) {
            EventContext end = row.enter(-1);
            end.setLogicWidth(end.logicWidth() + m_delta);
            row.setLogicWidth(width);
        }
    }
    int getLogicWidth(EventContext &context) override { return m_width + m_delta; }
    int getMinWidth(EventContext &context) override { return m_width; }
    void onEnterReflow(EventContext &context, Offset &offset) override {
        offset.x += 4;
        offset.y += m_top;
    }
    void onLeaveReflow(EventContext &context, Offset &offset) override {
        offset.x -= 4;
    }
    void onRelayout(EventContext &context, LayoutManager *sender) override {
        Buffer<uint32_t> MaxWidthBuffer;
        Buffer<uint32_t> MaxHeightBuffer;
        for_context(row, context) {
            for_context(col, row) {
                col.relayout();
                uint32_t width = col.minWidth();
                uint32_t height = col.minHeight();
                if (width > MaxWidthBuffer[col.index]) MaxWidthBuffer[col.index] = width;
                if (height > MaxHeightBuffer[row.index]) MaxHeightBuffer[row.index] = height;
            }
        }
        for_context(row, context) {
            for_context(col, row) {
                col.setLogicWidth(MaxWidthBuffer[col.index]);
                col.setLogicHeight(MaxHeightBuffer[row.index]);
            }
        }
        sender->reflow(context.enter(), true);
    }
    void onFocus(EventContext &context) override {
        CaretManager *caret = context.getCaretManager();
        Offset offset = caret->data().getOffset();
        GRect rect = context.rect();
        if (offset.y < rect.fTop) {
            context.enter().focus();
        } else {
            context.enter(-1).focus();
        }
    }

};
class FastRow : public RelativeElement {
    class FastText : public TextElement {
    public:
        void free() override {}
        Element *onNext(EventContext &context) override {
            return context.outer->cast<FastRow>()->get(++context.index);
        }
        Element *onPrev(EventContext &context) override {
            return context.outer->cast<FastRow>()->get(--context.index);
        }
        bool isHead(EventContext &context) override {
            return context.index == 0;
        }
        bool isTail(EventContext &context) override {
            return context.outer->cast<FastRow>()->m_items.size() == context.index + 1;
        }
    };
public:
    int m_width = 0;
    int m_height = 0;
    bool m_header = false;
    GColor m_color;
    std::vector<FastText> m_items;
    explicit FastRow(int column, int height, GColor color = SK_ColorWHITE) : m_color(color) {
        m_height = height;
        m_items.resize(column);
    }
    Tag getTag(EventContext &context) override {
        Tag tag = {_GT("Row Focus")};
        if (m_header) {
            tag.append(_GT(" Header Uneditable"));
        }
        return tag;
    }
    inline size_t size() { return m_items.size(); }
    inline Element *get(int index) {
        while (index < 0) {
            index += size();
        }
        if (index < 0 || index >= size()) {
            return nullptr;
        }
        return &m_items[index];
    }
    inline TextElement *getColumn(int index) { return &m_items[index]; }
    void setHeader(bool header) { m_header = header; }
    void setColor(GColor color) {
        m_color = color;
    }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        SkPaint paint;
        paint.setColor(m_color);
        canvas->drawRect(canvas.bound(0, 0), paint);
        if (context.isSelectedSelf()) {
            if (context.selectedCount() > 1) {
                canvas.drawRect(canvas.bound(0.5, 0.5), StyleTableBorderSelected);
            }
        }
        if (context.isSelectedRow()) {
            canvas.drawRect(canvas.bound(0.5, 0.5), StyleTableBorderSelected);
        }
        Root::onRedraw(context);
    }
    void onFocus(EventContext &context) override {
        CaretManager *caret = context.getCaretManager();
        Offset offset = caret->data().getOffset();
        for_context(ctx, context) {
            GRect rect = ctx.rect();
            if ((float) offset.x > rect.left() && (float) offset.x < rect.right()) {
                ctx.focus();
                return;
            }
        }
        context.enter().focus();
    }
    Display getDisplay() override { return DisplayBlock; }
    Element *getHead() override { return &m_items.front(); }
    Element *getTail() override { return &m_items.back(); }
    void onEnter(EventContext &context, EventContext &enter, int idx) override {
        while (idx < 0) idx += size();
        enter.index = idx;
        enter.element = get(idx);
    }
    int getLineNumber() override { return 0; }
    void onRelayout(EventContext &context, LayoutManager *sender) override {
        Offset offset;
        for_context(ctx, context) {
            ctx.current()->setLogicOffset(offset);
            offset.x += ctx.logicWidth();
        }
        m_width = offset.x;
    }
    int getLogicWidth(EventContext &context) override { return m_width; }
    int getLogicHeight(EventContext &context) override { return m_height; }
    int getWidth(EventContext &context) override { return m_width; }
    int getHeight(EventContext &context) override { return m_height; }
    void setLogicWidth(EventContext &context, int width) override { m_width = width; }
    void setLogicHeight(EventContext &context, int height) override { m_height = height; }
    void onSelectionDelete(EventContext &context, SelectionState state) override {
        if (context.tag().contain(_GT("Header"))) {
            return;
        }
        if (state == SelectionSelf && context.selectedCount() == 1) {
            return;
        }
        context.push(CommandType::DeleteElement, CommandData(this));
        separate(context, m_next, m_prev);
        if (state == SelectionStart) {

        }
        if (state == SelectionEnd) {
            //context.getDocContext()->clearSelect();
        }
    }
    void onUndo(Command command) override {
        Element::onUndo(command);
        if (command.type == CommandType::DeleteElement) {
            command.context->outer->update();
        }
    }
};
class FastTable : public Container<DisplayTable> {
public:
    int m_delta = 0;
    int m_top;
    int m_rowHeight = 23;
    int m_column = 0;
    explicit FastTable(int row, int column, int top = 5) : m_top(top) {
        m_column = column;
        for (int i = 0; i < row; ++i) {
            Container::append(new FastRow(column, m_rowHeight));
        }
    }
    Tag getTag(EventContext &context) override { return {_GT("Table Focus")}; }
    FastRow *addRow(int column) {
        if (column > m_column) {
            m_column = column;
        }
        auto *row = new FastRow(column, m_rowHeight);
        append(row);
        return row;
    }
    FastRow *getRow(int line) { return (FastRow *) get(line); }
    template <typename Type = TextElement>
    Type *getItem(int row, int col) { return (Type *) ((FastRow *) get(row))->get(col); }
    void onNotify(EventContext &context, int type, NotifyValue param, NotifyValue other) override {
        if (context.outer && context.outer->display() == DisplayRow) {
            context.outer->notify(type, param, other);
        } else {
            context.update();
        }
    }
    void setLogicWidth(EventContext &context, int width) override {
        m_delta = width - m_width;
        for_context(row, context) {
            EventContext end = row.enter(-1);
            end.setLogicWidth(end.logicWidth() + m_delta);
            row.setLogicWidth(width);
        }
    }
    int getLogicWidth(EventContext &context) override { return m_width + m_delta; }
    int getMinWidth(EventContext &context) override { return m_width; }
    void onEnterReflow(EventContext &context, Offset &offset) override {
        offset.x += 4;
        offset.y += m_top;
    }
    void onLeaveReflow(EventContext &context, Offset &offset) override {
        offset.x -= 4;
    }
    void onFinishReflow(EventContext &context, Offset &offset, LayoutContext &layout) override {
        Container::onFinishReflow(context, offset, layout);
        for_context(row, context) {
            int width = row.logicWidth();
            if (m_width > width) {
                EventContext ctx = row.enter(-1);
                ctx.setLogicWidth(ctx.logicWidth() + m_width - width);
            }
            row.setLogicWidth(m_width);
        }
    }
    void onRelayout(EventContext &context, LayoutManager *sender) override {
        for (int i = 0; i < m_column; ++i) {
            int columnMinWidth = 0;
            for_context(row, context) {
                EventContext column = row.enter(i);
                if (column.empty()) {
                    continue;
                }
                if (column.isTail() && i < m_column) {
                    continue;
                }
                int width = column.minWidth();
                if (width > columnMinWidth) {
                    columnMinWidth = width;
                }
            }
            for_context(row, context) {
                EventContext column = row.enter(i);
                if (column.empty()) {
                    continue;
                }
                if (column.isTail() && i < m_column) {
                    column.setLogicWidth(column.minWidth());
                } else {
                    column.setLogicWidth(columnMinWidth);
                }
                column.setLogicHeight(m_rowHeight);

            }
        }
        sender->reflow(context.enter(), true);
    }
    void onFocus(EventContext &context) override {
        CaretManager *caret = context.getCaretManager();
        Offset offset = caret->data().getOffset();
        GRect rect = context.rect();
        if (offset.y < rect.fTop) {
            context.enter().focus();
        } else {
            context.enter(-1).focus();
        }
    }
};
class CodeBlockElement : public Container<> {
public:
    explicit CodeBlockElement(int num) {
        for (int i = 0; i < num; ++i) {
            Container::append(new AutoLineElement());
        }
    }
    void onRelayout(EventContext &context, LayoutManager *sender) override {
        sender->reflow(context.enter(), true, {25, 0});
    }
    Tag getTag(EventContext &context) override { return _GT("CodeBlock"); }
    int getWidth(EventContext &context) override { return Element::getWidth(context); }
    void onBlur(EventContext &context, EventContext *focus, bool force) override {
        context_on_outer(context, Blur, focus, force);
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

    void onSelectionDelete(EventContext &context, SelectionState state) override {
        Element::onSelectionDelete(context, state);
        if (state == SelectionInside || state == SelectionRow) {
            context.push(CommandType::DeleteElement, CommandData(this));
            separate(context, m_next, m_prev);
        }

        if (state == SelectionStart) {
            auto first = context.enter().getSelectionState();
            if (first == SelectionStart) {
                // 不一定完全删除
            }
        }

        if (state == SelectionEnd) {
        }
    }

    void onUndo(Command command) override {
        if (command.type == CommandType::Separate) {
            if (m_head) {
                m_head->setPrev(nullptr);
            }
            if (m_tail) {
                m_tail->setNext(nullptr);
            }
        }
        Element::onUndo(command);
    }
};
class SwitchElement : public Container<> {
public:
    explicit SwitchElement(int num) {
        for (int i = 0; i < num; ++i) {
            Container::append(new CodeBlockElement(2));
        }
    }
    Tag getTag(EventContext &context) override { return {_GT("Switch Condition CodeBlock")}; }
    CodeBlockElement *addBlock(int num) { return (CodeBlockElement *) append(new CodeBlockElement(num)); }
private:
    int getWidth(EventContext &context) override { return Element::getWidth(context); }
public:
    void onFocus(EventContext &context) override {}
    void onBlur(EventContext &context, EventContext *focus, bool force) override {
        if (force) { return; }
        if (focus && focus->include(&context)) {
            return;
        }
        bool clear = true;
        for_context(block, context) {
            for_context(row, block) {
                if (!row.getLineViewer().empty()) {
                    clear = false;
                }
            }
        }
        if (clear) {
            context.replace(new AutoLineElement());
            if (context.outer) {
                context.outer->update();
            }
        }
    }
    void onRedraw(EventContext &context) override {
        Container::onRedraw(context);
        SkPaint paint;
        GScalar inter[2] = {3, 2};
        paint.setPathEffect(SkDashPathEffect::Create(inter, 2, 25))->unref();
        paint.setColor(SK_ColorBLACK);
        paint.setAlpha(110);
        int runawayTop = 15;
        int runawayLeft = 14;
        constexpr int runawayRight = 23;
        constexpr int lineLeft = 5;
        constexpr int lineRight = 23;
        constexpr float length = 5.7;
        for_context(ctx, context) {
            Canvas canvas = ctx.getCanvas();
            int lineTop = 0;
            if (ctx.isHead()) {
                lineTop = 15;
                if (context.nearby(-1).tag().contain(_GT("CodeBlock"))
                    || (context.parent().tag() == _GT("CodeBlock") && context.isHead())) {
                    lineTop = runawayTop = 20;
                }
                if (ctx.isTail()) {
                    runawayLeft = lineLeft;
                    canvas->drawLine(lineLeft, runawayTop, lineRight, runawayTop, paint);
                } else {
                    Offset offset = ctx.current()->getLogicOffset();
                    runawayTop = offset.y + ctx.height() - 10;
                }
            } else {
                lineTop = 20;
            }
            if (ctx.isTail()) {
                break;
            }
            int lineBottom = ctx.height() + 15;
            int underlineRight = lineRight;
            if (ctx.nearby(1).enter().tag().contain(_GT("CodeBlock"))) {
                underlineRight += 25;
            }
            canvas->drawLine(lineLeft, lineTop, lineRight, lineTop, paint); // 上横线
            canvas->drawLine(lineLeft, lineTop, lineLeft, lineBottom, paint); // 竖线
            canvas->drawLine(lineLeft, lineBottom, underlineRight, lineBottom, paint); // 下横线
            canvas->drawLine(runawayLeft, ctx.height() - 10, lineRight, ctx.height() - 10, paint);
            // 逃逸线横线
            GPath path = PathUtil::triangleRight(length, underlineRight, lineBottom);
            canvas->drawPath(path, paint);// 下边线三角形
        }
        // 需要绘制逃逸线
        Canvas canvas = context.getCanvas();
        if (context.nearby(1).tag().contain(_GT("CodeBlock"))) {
            int runawayBottom = context.height() + 12;
            canvas->drawLine(runawayLeft, runawayTop, runawayLeft, runawayBottom, paint); // 逃逸线竖线
            canvas->drawLine(runawayLeft, runawayBottom, runawayRight, runawayBottom, paint); // 逃逸线下横线
            GPath path = PathUtil::triangleRight(length, runawayRight, runawayBottom); // 右三角
            canvas->drawPath(path, paint);
        } else {
            int runawayBottom = context.height() - 2;
            canvas->drawLine(runawayLeft, runawayTop, runawayLeft, context.height(), paint); // 逃逸线竖线
            // 逃逸线向下
            GPath path = PathUtil::triangleDown(length, runawayLeft, runawayBottom);
            // 有可能后面还有流程语句 需要连接
            canvas->drawPath(path, paint);
        }

    }
    void onUndo(Command command) override {
        Element::onUndo(command);
    }
    void onSelectionDelete(EventContext& context, SelectionState state) override {
        Element::onSelectionDelete(context, state);
        if (state == SelectionInside || state == SelectionRow) {
            context.push(CommandType::DeleteElement, CommandData(this));
            separate(context, m_next, m_prev);
        }

        if (state == SelectionStart) {
            auto first = context.enter().getSelectionState();
            if (first == SelectionStart) {
                // 不一定完全删除
            }
        }


        if (state == SelectionEnd) {
            auto last = context.enter(-1).getSelectionState();
            if (last == SelectionEnd) {
                // 不一定完全删除...
            }
        }
    }
};
class SingleBlockElement : public CodeBlockElement {
public:
    using CodeBlockElement::CodeBlockElement;
    Tag getTag(EventContext &context) override { return _GT("Single Condition CodeBlock"); }
    void onBlur(EventContext &context, EventContext *focus, bool force) override {
        if (force) { return; }
        if (focus && focus->include(&context)) {
            return;
        }
        bool clear = true;
        for_context(ctx, context) {
            if (!ctx.getLineViewer().empty()) {
                clear = false;
            }
        }
        if (clear) {
            context.replace(new AutoLineElement());
            context.outer->update();
        }
    }
    void onRedraw(EventContext &context) override {
        CodeBlockElement::onRedraw(context);
        Canvas canvas = context.getCanvas();
        SkPaint paint;
        GScalar inter[2] = {3, 2};
        paint.setPathEffect(SkDashPathEffect::Create(inter, 2, 25))->unref();
        paint.setColor(SK_ColorBLACK);
        paint.setAlpha(110);
        constexpr int lineLeft = 5;
        constexpr int lineRight = 23;

        int lineTop = 15;
        if ((context.parent().tag() == _GT("CodeBlock") && context.isHead()) ||
            context.nearby(-1).tag().contain(_GT("CodeBlock"))) {
            lineTop = 20;
        }
        canvas->drawLine(lineLeft, lineTop, lineRight, lineTop, paint);
        if (context.nearby(1).tag().contain(_GT("CodeBlock"))) {
            int lineBottom = context.height() + 12;
            canvas->drawLine(lineLeft, lineTop, lineLeft, lineBottom, paint); // 竖线
            canvas->drawLine(lineLeft, lineBottom, lineRight, lineBottom, paint); // 下边线
            GPath path = PathUtil::triangleRight(5.7, lineRight, lineBottom);
            canvas->drawPath(path, paint);
        } else {
            int lineBottom = context.height() - 2;
            canvas->drawLine(lineLeft, lineTop, lineLeft, lineBottom, paint); // 竖线
            GPath path = PathUtil::triangleDown(5.7, lineLeft, lineBottom);
            canvas->drawPath(path, paint);
        }
    }
};
class LoopBlockElement : public SingleBlockElement {
public:
    using SingleBlockElement::SingleBlockElement;
    Tag getTag(EventContext &context) override { return _GT("Loop CodeBlock"); }
    void onRedraw(EventContext &context) override {
        CodeBlockElement::onRedraw(context);
        Canvas canvas = context.getCanvas();
        SkPaint paint;
        GScalar inter[2] = {3, 2};
        paint.setPathEffect(SkDashPathEffect::Create(inter, 2, 25))->unref();
        paint.setColor(SK_ColorBLACK);
        paint.setAlpha(110);
        constexpr int lineLeft = 5;
        constexpr int lineRight = 23;
        int lineTop = 15;
        if ((context.parent().tag() == _GT("CodeBlock") && context.isHead()) ||
            context.nearby(-1).tag().contain(_GT("Condition"))) {
            lineTop = 22;
        }
        int lineBottom = context.height() - 10;
        canvas->drawLine(lineLeft, lineTop, lineRight, lineTop, paint); // 上横线
        canvas->drawLine(lineLeft, lineTop, lineLeft, lineBottom, paint); // 竖线
        canvas->drawLine(lineLeft, lineBottom, lineRight, lineBottom, paint); // 下横线
        GPath path = PathUtil::triangleRight(5.7, lineRight, lineTop);
        canvas->drawPath(path, paint);
    }
};
class SubElement : public Container<> {
public:
    FastTable *header = nullptr;
    FastTable *locals = nullptr;
    int paramCount = 0;
    int localCount = 0;
    SubElement() {
        header = new FastTable(2, 4);
        auto *row = header->getRow(0);
        row->setHeader(true);
        row->setColor(SkColorSetRGB(230, 237, 228));
        row->getColumn(0)->m_data.append(_GT("Function Name"));
        row->getColumn(1)->m_data.append(_GT("Type"));
        row->getColumn(2)->m_data.append(_GT("Public"));
        row->getColumn(3)->m_data.append(_GT("Comment  "));
        Container::append(header);
        locals = new FastTable(0, 4);
        Container::append(locals);
    }
    GString &content(int col) { return header->getItem(1, col)->m_data; }
    void addParam(const GChar *name, const GChar *type) {
        if (paramCount == 0) {
            auto *param_header = header->addRow(6);
            param_header->setHeader(true);
            param_header->setColor(SkColorSetRGB(230, 237, 228));
            param_header->getColumn(0)->m_data.append(_GT("Name"));
            param_header->getColumn(1)->m_data.append(_GT("Type"));
            param_header->getColumn(2)->m_data.append(_GT("Nullable"));
        }
        paramCount++;
        auto *row = header->addRow(6);
        row->getColumn(0)->m_data.append(name);
        row->getColumn(1)->m_data.append(type);
    }
    void addLocal(const GChar *name, const GChar *type) {
        if (localCount == 0) {
            //
            auto *local_header = locals->addRow(4);
            local_header->setHeader(true);
            local_header->setColor(SkColorSetRGB(217, 227, 240));
            local_header->getColumn(0)->m_data.append(_GT("Name   "));
            local_header->getColumn(1)->m_data.append(_GT("Type   "));
        }
        localCount++;
        auto *row = locals->addRow(4);
        row->getColumn(0)->m_data.append(name);
        row->getColumn(1)->m_data.append(type);
    }
    Tag getTag(EventContext &context) override { return {_GT("SubElement")}; }
public:
    int getWidth(EventContext &context) override { return Element::getWidth(context); }

    void onSelectionDelete(EventContext &context, SelectionState state) override {
        Element::onSelectionDelete(context, state);
        if (state == SelectionSelf) {
            context.getDocContext()->clearSelect();
            context.update();
        }

    }
};
class MultiLine : public RelativeElement {
private:
    class SingleLine : public SyntaxLineElement {
    public:
        int count = 6;
        Tag getTag(EventContext &context) override { return {_GT("SingleLine Focus")}; }
        Display getDisplay() override { return DisplayLine; }
        Element *onNext(EventContext &context) override {
            if (++context.index >= count) return nullptr;
            return this;
        }
        Element *onPrev(EventContext &context) override {
            if (--context.index < 0) return nullptr;
            return this;
        }
        Offset getOffset(EventContext &context) override {
            while (context.index < 0) context.index = count + context.index;
            return context.outer->offset() + Offset{20, context.index * 25 + 20};
        }
        int getLogicHeight(EventContext &context) override { return 25; }
        void insert(EventContext &context) override {
            context.insertLine(1);
            count++;
        }
        void erase(EventContext &context) override {
            context.deleteLine();
            count--;
        }
        void onRedraw(EventContext &context) override {
            SyntaxLineElement::onRedraw(context);
            Canvas canvas = context.getCanvas();
            std::string &&line = std::to_string(context.index + 1);
            SkPaint paint;
            canvas->drawText(line.c_str(), line.length(), -15, 20, paint);
        }
        void free() override {}
    };
    SingleLine m_lines;
public:
    Element *getHead() override {if (!expand) return nullptr; return &m_lines; }
    Element *getTail() override {if (!expand) return nullptr; return &m_lines; }
    Display getDisplay() override { return DisplayBlock; }
    int getLogicHeight(EventContext &context) override { return m_lines.count * 25; }
    bool expand = true;
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        canvas.drawText(_GT("Inline C Language"), 17 * 2, 18, 15, StyleDeafaultFont);
        SkPaint border;
        border.setStyle(SkPaint::Style::kStroke_Style);
        border.setColor(SK_ColorLTGRAY);
        canvas->drawRect(canvas.bound(0.5, 0.5), border);
        canvas->drawLine(0, 20, context.width(), 20, border);

        border.setStyle(SkPaint::kFill_Style);
        if (expand) {
            canvas->drawPath(PathUtil::triangleDown(8, 9, 8), border);
            Root::onRedraw(context);
        } else {
            canvas->drawPath(PathUtil::triangleRight(8, 7, 10), border);
        }
    }
    int getLineNumber() override { return m_lines.count; }
    int getChildCount() override { return m_lines.count; }
    int getHeight(EventContext &context) override { return expand ? Root::getHeight(context) + 25 : 20; }
    void onEnterReflow(EventContext &context, Offset &offset) override {
        offset.x += 5;
    }
    void onLeaveReflow(EventContext &context, Offset &offset) override {
        offset.x -= 5;
    }
    void onLeftButtonUp(EventContext &context, int x, int y) override {
        if (context.relative(x, y).y < 20) {
            expand = !expand;
            context.reflow();
            if (context.getCaretManager()->include(&context)) {
                if (expand) {
                    context.enter().focus();
                } else {
                    context.focus();
                }
            }
            context.redraw();
        }
    }
    void onFocus(EventContext &context) override {
        context.getCaretManager()->set(155, 3);
    }
    void onBlur(EventContext &context, EventContext *focus, bool force) override {

    }
};
class ModuleElement : public Container<> {
public:
    ModuleElement()= default;
    GString name;
    bool expand = false;
    bool layouted = false;
    Tag getTag(EventContext &context) override { return {_GT("SubElement")}; }
public:
    int getWidth(EventContext &context) override { return Element::getWidth(context); }
    int getHeight(EventContext &context) override { return expand ? Container<>::getHeight(context) + 15 : 20; }
    void onFocus(EventContext &context) override {
        context.getCaretManager()->set(15, 3);
    }
    void onLeftButtonUp(EventContext &context, int x, int y) override {
        if (context.relative(x, y).y < 20) {
            expand = !expand;
            if (expand && !layouted) {
                context.relayout();
                layouted = true;
            }
            context.focus();
            context.reflow();
            context.redraw();
        }
        Element::onLeftButtonUp(context, x, y);
    }
    void onRelayout(EventContext &context, LayoutManager *sender) override {
        if (expand) {
            sender->reflow(context.enter(), true, {5, 30});
        }
    }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        canvas.drawText(name.c_str(), name.size() * sizeof(GChar), 18, 15, StyleDeafaultFont);
        SkPaint border;
        border.setStyle(SkPaint::Style::kStroke_Style);
        border.setColor(SK_ColorLTGRAY);
        canvas->drawRect(canvas.bound(0.5, 0.5), border);
        canvas->drawLine(0, 20, context.width(), 20, border);
        border.setStyle(SkPaint::kFill_Style);
        if (expand) {
            canvas->drawPath(PathUtil::triangleDown(8, 9, 8), border);
            Root::onRedraw(context);
        } else {
            canvas->drawPath(PathUtil::triangleRight(8, 7, 10), border);
        }
    }
    void onEnterReflow(EventContext &context, Offset &offset) override {
        offset.y += 10;
    }
    void onSelectionDelete(EventContext &context, SelectionState state) override {
        bool reflow = false;
        for_context(ctx, context) {
            SelectionState selection = ctx.getSelectionState();
            if (selection != SelectionNone) {
                if (selection == SelectionStart) {
                    reflow = true;
                }
                ctx.current()->onSelectionDelete(ctx, selection);
            }
        }
        if (reflow) {
            context.relayout();
            context.getDocContext()->clearSelect();
        }
    }
};

#endif //GEDITOR_TABLE_H
