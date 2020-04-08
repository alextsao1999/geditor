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
#define TAG_FOCUS TAG("Focus")
#define TAG_LINE TAG("Line")
#define TAG_TABLE TAG("Table")
#define TAG_HEADER TAG("Header")
#define TAG_UNEDITABLE TAG("Uneditable")
#define TAG_UNDELETEABLE TAG("Undeleteable")
#define context_on_outer(ctx, method, ...) if ((ctx).outer) context_on(*((ctx).outer), method, ##__VA_ARGS__);
class DrawUtil {
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
    static void DrawSlection(EventContext &context, Offset start, Offset end) {
        if (!context.getDocContext()->hasSelection()) {
            return;
        }
        Canvas canvas = context.getCanvas();
        GRect rect;
        rect.set({(GScalar) start.x, (GScalar) start.y}, {(GScalar) end.x, (GScalar) end.y});
        rect.fBottom = rect.fTop + context.getStyle().getTextSize() + 4;
        SkPaint paint;
        paint.setColor(SkColorSetRGB(218, 227, 233));
        paint.setAlpha(255);
        paint.setAntiAlias(true);
        canvas->drawRect(rect, paint);
        //canvas->drawRoundRect(rect, 4, 4, paint);
        //paint.setColor(SK_ColorBLACK);
        //paint.setAlpha(30);
        //paint.setStyle(SkPaint::kStroke_Style);
        //rect.inset(0.5f, 0.5f);
        //canvas->drawRoundRect(rect, 4, 4, paint);
        //canvas->drawRect(rect, paint);
    }
};
using RectVec = Buffer<SkRect>;
class TextCaretService {
protected:
    Offset m_offset;
    Offset m_move;
    EventContext* m_context = nullptr;
    int m_index = 0; // 没有匹配到m_move 就直接设置index
public:
    TextCaretService(const Offset& offset, EventContext* context) : m_offset(offset), m_context(context) {
        m_index = m_context->getCaretManager()->data().index;
        m_move = m_context->relOffset(m_context->getCaretManager()->data().offset);
    }
    void setTextOffset(Offset offset) { m_offset = offset; }
    int& index() { return m_index; }
    void moveLeft() { moveToIndex(m_index - 1); }
    void moveRight() { moveToIndex(m_index + 1); }
    void moveToIndex(int index) {
        m_index = index;
        m_move = {0, 0};
    }
    void moveTo(Offset offset) {
        m_index = 0;
        m_move = offset;
    }
    bool commit(int style = StyleDeafaultFont) {
        LineViewer viewer = m_context->getLineViewer();
        return commit(viewer.c_str(), viewer.size(), m_context->getStyle(style));
    }
    bool commit(GString &string, int style = StyleDeafaultFont) {
        return commit(string.c_str(), string.length() * sizeof(GChar), m_context->getStyle(style));
    }
    bool commit(const void* text, size_t bytelength, GStyle& paint, SkRect* bound = nullptr) {
        bool res; // 是否未越界
        int index = m_index;
        int length = paint.countText(text, bytelength);
        Buffer<SkScalar> widths(length);
        RectVec rects(length);
        paint.getTextWidths(text, bytelength, widths.data(), rects.data());
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
        } else {
            index = length;
            if (length == 0) {
                caret.x = m_offset.x;
            } else {
                if (rects[length - 1].width() == 0) {
                    rects[length - 1].fRight += widths[length - 1];
                }
                caret.x = rects[length - 1].right();
            }
        }
        caret.y = m_offset.y;
        show(caret, index);
        if (res) {
            m_move = Offset(0, 0);
        }
        return res;
    }
    void show(Offset offset, int index) {
        CaretManager *m_caret = m_context->getCaretManager();
        m_caret->data().set(index, m_context->absOffset(offset));
        m_caret->set(offset);
    }
    bool breakText(const void *text,  size_t bytelength, GStyle &paint, RectVec &rects) {
        int length = paint.countText(text, bytelength);
        if (length <= 0) {
            return false;
        }
        Buffer<SkScalar> widths(length);
        rects.resize(length);
        paint.getTextWidths(text, bytelength, widths.data(), rects.data());
        SkScalar size = paint.getTextSize();
        int width = 0;
        for (int i = 0; i < length; ++i) {
            rects[i].offset(m_offset.x + width, size + (SkScalar) m_offset.y);
            width += widths[i];
        }
        if (rects.back().width() == 0) {
            rects[length - 1].fRight += widths[length - 1];
        }
        return true;
    }
    bool breakText(RectVec &rects, int style = StyleDeafaultFont) {
        LineViewer viewer = m_context->getLineViewer(0);
        return breakText(viewer.c_str(), viewer.size(), m_context->getStyle(style), rects);
    }
    CaretPos getCaretPos(RectVec &rects, int index) {
        while (index < 0) {
            index = rects.size() + index + 1;
        }
        if (rects.size() == 0) {
            return {index, m_offset};
        }
        if (index >= rects.size()) {
            index = rects.size();
            return {index, m_context->absOffset(Offset(rects.back().right(), m_offset.y))};
        }
        return {index, m_context->absOffset(Offset(rects[index].left(), m_offset.y))};
    }
    Offset getRelOffset(RectVec &rects, int index) {
        int length = rects.size();
        if (length <= 0) {
            return m_offset;
        }
        while (index < 0) {
            index = length + index + 1;
        }
        Offset caret;
        if (index < length) {
            caret.x = rects[index].left();
        } else {
            caret.x = rects.back().right();
        }
        caret.y = m_offset.y;
        return caret;
    }
    Offset getAbsOffset(RectVec &rects, int index) {
        return m_context->absOffset(getRelOffset(rects, index));
    }
    int getIndex(RectVec &rects, int x, int y) {
        Offset offset = Offset{x, y} - m_context->offset();
        int index = 0;
        for (int i = 0; i < rects.size(); ++i) {
            if (SkIntToScalar(offset.x) >= rects[i].x()) {
                if (SkIntToScalar(offset.x) > rects[i].centerX()) {
                    index = i + 1;
                } else {
                    index = i;
                }
            }
        }
        while (index < 0) {
            index = rects.size() + index + 1;
        }
        return index;
    }
    static int GetIndex(EventContext &context, Offset offset, int x, int y) {
        TextCaretService service(offset, &context);
        RectVec rects;
        service.breakText(rects);
        return service.getIndex(rects, x, y);
    }
    static CaretPos GetCaretPos(EventContext &context, Offset offset, int x, int y) {
        TextCaretService service(offset, &context);
        RectVec rects;
        service.breakText(rects);
        return service.getCaretPos(rects, service.getIndex(rects, x, y));
    }
    static CaretPos GetCaretPos(EventContext &context, Offset offset, int index) {
        TextCaretService service(offset, &context);
        RectVec rects;
        service.breakText(rects);
        return service.getCaretPos(rects, index);
    }
};
class OffsetCaretService {
public:
    EventContext *m_context;
    Offset m_move;
    explicit OffsetCaretService(EventContext *context) : m_context(context) {
        m_move = m_context->relOffset(m_context->getCaretManager()->data().offset);
    }
    OffsetCaretService &moveTo(Offset offset) {
        m_move = offset;
        return *this;
    }
    void commit(Offset offset) {
        m_move = offset;
        commit();
    }
    void commit() {
        auto *caret = m_context->getCaretManager();
        caret->data().setOffset(m_move);
        caret->set(m_move);
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
    Display getDisplay(EventContext &context) override { return DisplayBlock; }
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
    Display getDisplay(EventContext &context) override { return DisplayBlock; }
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
    inline Offset padding() { return {4, 6}; }
    inline Offset location() { return {4, 6}; }
    virtual void insert(EventContext &context) {
        context.insert(copy());
        int flags = context.getLineViewer().flags();
        if ((flags & LineFlagLineVert) || (flags & LineFlagFold)) {
            context.getLineViewer(1).flags() |= LineFlagLineVert;
        }
    }
    virtual void erase(EventContext &context) {
        int flags = context.getLineViewer().flags();
        if (flags & LineFlagLineHorz) {
            int &prev = context.getLineViewer(-1).flags();
            if (prev & LineFlagLineVert) {
                prev &= ~LineFlagLineVert;
                prev |= LineFlagLineHorz;
            } else {
                prev &= ~LineFlagFold;
            }
        }
        context.remove();
//        context.prevLine(getLineNumber());
    }
    virtual Element *copy() { return new LineElement(); }
    virtual void drawSelection(EventContext &context) {
        SelectionState state = context.getSelectionState();
        if (state == SelectionSelf) {
            Offset start = context.relOffset(context.getDocContext()->getSelectStart());
            Offset end = context.relOffset(context.getDocContext()->getSelectEnd());
            DrawUtil::DrawSlection(context, start, end);
        }
        if (state == SelectionStart) {
            TextCaretService text(padding(), &context);
            RectVec rects;
            text.breakText(rects);
            Offset start = context.relOffset(context.getDocContext()->getSelectStart());
            if (rects.size()) {
                DrawUtil::DrawSlection(context, start, Offset(rects.back().right(), rects.back().top()));
            } else {
                if (!context.isFocusIn())
                    DrawUtil::DrawSlection(context, padding(), Offset(10, 6));
            }
        }
        if (state == SelectionEnd) {
            Offset end = context.relOffset(context.getDocContext()->getSelectEnd());
            if (context.getLineViewer().length()) {
                DrawUtil::DrawSlection(context, padding(), end);
            } else {
                if (!context.isFocusIn())
                    DrawUtil::DrawSlection(context, padding(), Offset(10, 6));
            }
        }
        if (state == SelectionInside || state == SelectionRow) {
            TextCaretService text(padding(), &context);
            RectVec rects;
            text.breakText(rects);
            if (rects.size()) {
                DrawUtil::DrawSlection(context, padding(), Offset(rects.back().right(), rects.back().top()));
            } else {
                DrawUtil::DrawSlection(context, padding(), Offset(10, 6));
            }
        }
    }
public:
    Tag getTag(EventContext &context) override {
        if (context.getLineViewer().flags() & LineFlagHide) {
            return Tag::Format(TAG("Line (%d)"), context.line() + 1);
        }
        return Tag::Format(TAG("Line Focus (%d)"), context.line() + 1);
    }
    int getLineNumber() override { return 1; }
    Display getDisplay(EventContext &context) override {
        if (context.getLineViewer().flags() & LineFlagHide) {
            return DisplayNone;
        }
        return DisplayBlock;
    }
    int getLogicHeight(EventContext &context) override { return 25; }
    int getLogicWidth(EventContext &context) override {
        auto line = context.getLineViewer();
        return (int) context.getStyle().measureText(line.c_str(), line.size()) + 8;
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        if (auto index = context.document()->margin()->index(context.absolute(x, y))) {
            auto line = context.getLineViewer();
            if (index == 2) {
                line.flags() ^= LineFlagBP;
            }
            if (index == 3 && ((line.flags() & LineFlagFold) || (line.flags() & LineFlagExpand))) {
                OnFold(context);
            }

        }
        context.pos().setOffset(context.absolute(x, y));
        context.focus();
        context.redraw();
    }
    void onLeftDoubleClick(EventContext &context, int x, int y) override {
        TextCaretService service(padding(), &context);
        auto line = context.getLineViewer();
        RectVec rects;
        service.breakText(rects);
        int length = rects.size();
        if (length == 0) {
            return;
        }
        int startIndex = service.index();
        int endIndex = service.index();
        while (startIndex - 1 >= 0 && (IsCodeChar(line.charAt(startIndex - 1)) || IsNumber(line.charAt(startIndex - 1)))) {
            startIndex--;
        }
        while (endIndex < length && (IsCodeChar(line.charAt(endIndex)) || IsNumber(line.charAt(endIndex)))) {
            endIndex++;
        }
        CaretPos start = service.getCaretPos(rects, startIndex);
        CaretPos end = service.getCaretPos(rects, endIndex);
        context.getDocContext()->select(start, end);
        context.setPos(end);
        context.focus();
        context.redraw();
    }
    void onMouseMove(EventContext &context, int x, int y) override {
        if (context.isSelecting()) {
            context.pos().setOffset(context.absolute(x, y));
            context.focus();
        }
/*
        if (context.isMouseIn()) {
            context.redraw();
        }
*/

    }
    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        TextCaretService service(padding(), &context);
        if (code == VK_DELETE) {
            auto line = context.getLineViewer();
            if (service.index() >= line.length()) {
                return;
            }
            line.remove(service.index());
            context.redraw();
        }
        if (code == VK_HOME) {
            auto line = context.getLineViewer();
            context.pos().setIndex(line.getSpaceCount());
            context.focus(false);
        }
        if (code == VK_END) {
            context.pos().setIndex(-1);
            context.focus(false);
        }
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
    void onInputChar(EventContext &context, SelectionState state, int ch) override {
        auto* caret = context.getCaretManager();
        TextCaretService service(padding(), &context);
        auto line = context.getLineViewer();
        switch (ch) {
            case VK_BACK:
                if (state == SelectionNone) {
                    if (service.index() > 0) {
                        line.remove(service.index() - 1);
                        service.moveLeft();
                    } else {
                        EventContext *prev = context.findPrev(TAG_FOCUS);
                        if (!prev) { return; }
                        if (prev->tag().contain(TAG_LINE)) {
                            int prevLength = prev->getLineViewer().length();
                            context.combineLine(-1);
                            erase(context);
                            prev->reflow();
                            prev->redraw();
                        } else {
                            prev->pos().setIndex(-1);
                        }
                        prev->focus(false);
                        return;
                    }
                }
                break;
            case VK_RETURN: {
                insert(context);
                context.breakLine(0, service.index());
                context.reflow();
                context.redraw();
                caret->data().setIndex(0);
                caret->next();
                return;
            }
            default:
                line.insert(service.index(), ch);
                service.moveRight();
                break;
        }
        service.commit();
        context.redraw();
    }
    void onFocus(EventContext &context) override {
        context_on_outer(context, Focus);
        TextCaretService service(padding(), &context);
        service.commit();
    }
    void onBlur(EventContext &context, EventContext *focus, bool force) override {
        context_on_outer(context, Blur, focus, force);
    }
    void onUndo(Command command) override {
        auto line = command.context->getLineViewer(0, false);
        if (command.type == CommandType::Break) {
            auto next = command.context->getLineViewer(1, false);
            line.append(next.c_str());
            next.clear();
        }
        if (command.type == CommandType::Combine) {
            command.context->breakLine(-1, command.data.value, false);
        }
        if (command.type == CommandType::AddChar) {
            line.remove(command.data.input.pos);
        }
        if (command.type == CommandType::AddString) {
            line.remove(command.data.input.pos, command.data.input.ch);
        }
        if (command.type == CommandType::SetString) {
            line.content().assign(*command.data.string.string);
            delete command.data.string.string;
        }
        if (command.type == CommandType::DeleteChar) {
            line.insert(command.data.input.pos, command.data.input.ch);
        }
        if (command.type == CommandType::DeleteString) {
            line.insert(command.data.string.pos, command.data.string.string->c_str());
            delete command.data.string.string;
        }
        command.context->setPos(command.pos);
        command.context->focus(true, true);
        Element::onUndo(command);
        if (command.type == CommandType::Break || command.type == CommandType::DeleteElement) {
            command.context->doc->undo();
        }
    }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        drawSelection(context);
        SkPaint border;
        border.setStyle(SkPaint::Style::kStroke_Style);
        border.setColor(SK_ColorLTGRAY);
        canvas->drawRect(canvas.bound(0.5, 0.5), border);
        LineViewer viewer = context.getLineViewer();
        canvas.translate(0, context.getStyle(StyleDeafaultFont).getTextSize());
        canvas.drawText(viewer.c_str(), viewer.size(), location().x, location().y);
    }
    void onSelectionDelete(EventContext &context, SelectionState state) override {
        auto line = context.getLineViewer();
        auto *dctx = context.getDocContext();
        //printf("state:%s str:%s\n", SelectionString[state], W2A(line.c_str()));
        if (state == SelectionSelf) {
            CaretPos start = context.getDocContext()->m_selectStartPos;
            CaretPos end = context.getDocContext()->m_selectEndPos;
            if (start.index > end.index) {
                std::swap(start, end);
            }
            int length = end.index - start.index;
            line.remove(start.index, length);
            context.setPos(start);
        }
        if (state == SelectionStart) {
            CaretPos start = context.getDocContext()->m_selectStartPos;
            int length = line.length() - start.index;
            line.remove(start.index, length);
            context.setPos(start);
            //context.focus();
        }
        if (state == SelectionInside || state == SelectionRow) {
            context.getLineViewer().clear();
            if (context.nearby(-1).tag().contain(TAG_TABLE)) {
                if (context.isTail()) {
                    return;
                }
            }
            erase(context);
        }
        if (state == SelectionEnd) {
            CaretPos end = context.getDocContext()->m_selectEndPos;
            line.remove(0, end.index);
            EventContext prev = context.nearby(-1);
            if (prev.tag().contain(TAG_LINE)) {
                context.combineLine(-1);
                prev.focus(true, true);
                erase(context);
            }
        }

    }
    void onSelectionToString(EventContext &context, SelectionState state, ostream &out) override {
        if (state == SelectionSelf) {
            CaretPos start = context.getDocContext()->m_selectStartPos;
            CaretPos end = context.getDocContext()->m_selectEndPos;
            int length = end.index - start.index;
            out.write(context.getLineViewer().content().c_str() + start.getIndex(), length);
        }
        if (state == SelectionStart) {
            CaretPos start = context.getDocContext()->m_selectStartPos;
            out << context.getLineViewer().content().c_str() + start.getIndex() << std::endl;
        }
        if (state == SelectionInside || state == SelectionRow) {
            out << context.getLineViewer().content() << std::endl;
        }
        if (state == SelectionEnd) {
            CaretPos end = context.getDocContext()->m_selectEndPos;
            out.write(context.getLineViewer().content().c_str(), end.getIndex());
        }

    }

public:
    static void OnFold(EventContext &context) {
        EventContext ctx = context;
        auto line = ctx.getLineViewer();
        if (line.flags() & LineFlagFold) {
            line.flags() ^= LineFlagFold;
            line.flags() |= LineFlagExpand;
        } else {
            line.flags() ^= LineFlagExpand;
            line.flags() |= LineFlagFold;
        }
        do {
            ctx.next();
            auto now = ctx.getLineViewer();
            if ((now.flags() & LineFlagLineVert) || (now.flags() & LineFlagLineHorz)) {
                now.flags() ^= LineFlagHide;
            } else {
                break;
            }
        } while (ctx.has());
        context.reflow();
        context.redraw();
    }
};
class SyntaxLineElement : public LineElement {
public:
    Element *copy() override { return new SyntaxLineElement(); }
    void onRedraw(EventContext &context) override {
        TextCaretService service(padding(), &context);
        RectVec rects;
        service.breakText(rects);

        context.gutter();
        Canvas canvas = context.getCanvas();
        drawSelection(context);

        SkPaint border;
        if (context.isSelectedRow()) {
            border.setStyle(SkPaint::Style::kStrokeAndFill_Style);
        } else {
            border.setStyle(SkPaint::Style::kStroke_Style);
        }
        border.setColor(SK_ColorLTGRAY);
        //canvas->drawRect(canvas.bound(0.5, 0.5), border);

        auto *lexer = context.getLexer();
        Offset offset = location();
        canvas.translate(offset.x, (float) offset.y + context.getStyle().getTextSize());
        while (lexer->has()) {
            Token token = lexer->next();
            if (token == TokenIdentifier && token.style < StyleKeywordFont) {
                if (lexer->peek() == _GT("(") || (lexer->peek() == TokenSpace && lexer->peek(2) == _GT("("))) {
                    token.style = StyleFunctionFont;
                }
            }
            GStyle &style = context.getStyle(token.style);
            canvas->drawText(token.c_str(), token.size(), 0, 0, style.paint());
            canvas.translate(style.measureText(token.c_str(), token.size()), 0);
        }
    }
};
class AutoLineElement : public SyntaxLineElement {
public:
    Element *copy() override { return new AutoLineElement(); }
public:
    void onInputChar(EventContext &context, SelectionState state, int ch) override;
    void onBlur(EventContext &context, EventContext *focus, bool force) override {
        if (context.document()->m_onBlur) {
            if (context.document()->m_onBlur(&context, (int) focus, (int) force)) {
                return;
            }
        }
        LineElement::onBlur(context, focus, force);
    }
    void onMouseHover(EventContext &context, int x, int y) override;
    void onMouseLeave(EventContext &context, int x, int y) override;
    void onRedraw(EventContext &context) override {
        SyntaxLineElement::onRedraw(context);
        if (context.getLineViewer().flags() & LineFlagExpand) {
            Canvas canvas = context.getCanvas();
            SkPaint paint;
            paint.setStyle(SkPaint::kStroke_Style);
            SkRect rect = SkRect::MakeWH(context.logicWidth(), context.height());
            rect.inset(0.5, 4);
            rect.offset(0, 3);
            canvas->drawRect(rect, paint);
        }
    }
};
class TextElement : public RelativeElement {
public:
    GString m_data;
    int m_min = 50;
    int m_width = 0;
    int m_height = 20;
    bool m_radio = false;
    explicit TextElement() = default;
    void setContent(const GChar* value) { m_data.assign(value); }
    inline Offset location() { return {6, 2}; }
public:
    Tag getTag(EventContext &context) override { return {TAG("Text Focus")}; }
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
    Display getDisplay(EventContext &context) override { return DisplayInline; }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.pos().setOffset(context.absolute(x, y));
        context.focus();
        context.redraw();
    }
    void onLeftDoubleClick(EventContext &context, int x, int y) override {
        if (m_radio) {
            if (m_data.empty()) {
                m_data.assign(_GT("1"));
            } else {
                m_data.clear();
            }
            context.redraw();
            return;
        }
        if (context.outer && context.outer->tag().contain(TAG_HEADER)) {
            return;
        }
        TextCaretService service(Offset(6, 2), &context);
        RectVec rects;
        service.breakText(m_data.c_str(), m_data.size() * sizeof(GChar), context.getStyle(StyleTableFont), rects);
        auto start = service.getCaretPos(rects, 0);
        auto end = service.getCaretPos(rects, -1);
        context.getDocContext()->select(start, end);
        context.setPos(end);
        context.getCaretManager()->getEventContext()->focus(false);
    }
    void onMouseMove(EventContext &context, int x, int y) override {
        if (context.isSelecting()) {
            context.pos().setOffset(context.absolute(x, y));
            context.focus();
        }
    }
    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        TextCaretService service(Offset(6, 2), &context);
        if (code == VK_LEFT) {
            if (context.outer->tag().contain(TAG_UNEDITABLE) || m_radio) {
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
            if (context.outer->tag().contain(TAG_UNEDITABLE) || m_radio) {
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
                if (real && real->tag().contain(TAG("Row"))) {
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
        if (code == VK_RETURN) {
            context_on_ptr(context.outer, onClone);
        }
    };
    void onInputChar(EventContext &context, SelectionState state, int ch) override {
        if (m_radio) {
            if (m_data.empty()) {
                m_data.assign(_GT("1"));
            } else {
                m_data.clear();
            }
            context.redraw();
            return;
        }
        if (context.outer->tag().contain(TAG_UNEDITABLE)) {
            return;
        }
        TextCaretService service(Offset(6, 2), &context);
        switch (ch) {
            case VK_BACK:
                if (service.index() > 0 && state == SelectionNone) {
                    int current = service.index() - 1;
                    context.push(CommandType::DeleteChar,
                                 CommandData(current, m_data.at(current)));
                    m_data.erase(current, 1);
                    service.moveLeft();
                }
                break;
            case VK_RETURN:
                break;
            default:
                context.push(CommandType::AddChar, CommandData(service.index(), ch));
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
        if (context.outer->tag().contain(TAG_UNEDITABLE) || m_radio) {
            context.setPos(CaretPos(0, context.absOffset(Offset(6, 2))));
            context.getCaretManager()->set(6, 2);
            return;
        }
        TextCaretService service(Offset(6, 2), &context);
        service.commit(m_data, StyleTableFont);
    }
    void onBlur(EventContext &context, EventContext *focus, bool force) override {
        if (context.doc->m_onBlur) {
            if (context.doc->m_onBlur(&context, (int) focus, (int) force)) {
                return;
            }
        }
        Element::onBlur(context, focus, force);
    }
    void onRedraw(EventContext &context) override;
    void onUndo(Command command) override {
        auto &line = m_data;
        if (command.type == CommandType::AddChar) {
            line.erase(command.data.input.pos);
        }
        if (command.type == CommandType::DeleteChar) {
            line.insert(line.begin() + command.data.input.pos, command.data.input.ch);
        }
        if (command.type == CommandType::DeleteString) {
            line.insert(command.data.string.pos, *command.data.string.string);
            delete command.data.string.string;
        }
        command.context->outer->notify(Update, 0, 0);
        command.context->setPos(command.pos);
        command.context->focus(true, true);
        Element::onUndo(command);
    }
    void onSelectionDelete(EventContext &context, SelectionState state) override {
        auto &line = m_data;
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
            line.erase(start.index, length);
            context.setPos(start);
        }
    }

};
class NormalRowElement : public Container<DisplayRow> {
public:
    GColor m_color;
    explicit NormalRowElement(int column, GColor color = SK_ColorTRANSPARENT) : m_color(color) {
        for (int i = 0; i < column; ++i) {
            Container::append(new TextElement());
        }
    }
public:
    Tag getTag(EventContext &context) override { return {TAG("Row Focus")}; }
    void setColor(GColor color) {
        m_color = color;
    }
    void onNotify(EventContext &context, int type, NotifyParam param, NotifyValue other) override {
        context.outer->notify(type, param, other);
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
class NormalTableElement : public Container<DisplayTable> {
public:
    int m_delta = 0;
    int m_top;
    NormalTableElement(int line, int column, int top = 5) {
        m_top = top;
        for (int i = 0; i < line; ++i) {
            Container::append(new NormalRowElement(column));
        }
    }
    void replace(int line, int column, Element *element) {
        auto *row = (Container *) get(line);
        row->replace(column, element)->free();
    }
public:
    Tag getTag(EventContext &context) override { return {TAG("Table Focus")}; }
    int getLogicWidth(EventContext &context) override { return m_width + m_delta; }
    int getMinWidth(EventContext &context) override { return m_width; }
    void setLogicWidth(EventContext &context, int width) override {
        m_delta = width - m_width;
        for_context(row, context) {
            EventContext end = row.enter(-1);
            end.setLogicWidth(end.logicWidth() + m_delta);
            row.setLogicWidth(width);
        }
    }
    void onNotify(EventContext &context, int type, NotifyParam param, NotifyValue other) override {
        if (context.outer && context.outer->display() == DisplayRow) {
            context.outer->notify(type, param, other);
        } else {
            context.update();
        }
    }
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
    bool m_undeleteable = false;
    std::vector<FastText> m_items;
    explicit FastRow(int column) {
        m_items.resize(column);
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
    inline TextElement *getColumn(int index) { return (TextElement *) get(index); }
    void setHeader(bool header) { m_header = header; }
    void setUndeleteable(bool ud) { m_undeleteable = ud; }
public:
    Tag getTag(EventContext &context) override {
        Tag tag = {TAG("Row Focus")};
        if (m_header) {
            tag.append(TAG(" Header Uneditable Undeleteable"));
        }
        if (m_undeleteable) {
            tag.append(TAG(" Undeleteable"));
        }
        return tag;
    }
    Display getDisplay(EventContext &context) override { return DisplayBlock; }
    int getLineNumber() override { return 0; }
    Element *getHead() override { return get(0); }
    Element *getTail() override { return get(-1); }
    int getLogicWidth(EventContext &context) override { return m_width; }
    int getLogicHeight(EventContext &context) override { return m_height; }
    int getWidth(EventContext &context) override { return m_width; }
    int getHeight(EventContext &context) override { return m_height; }
    void setLogicWidth(EventContext &context, int width) override { m_width = width; }
    void setLogicHeight(EventContext &context, int height) override { m_height = height; }
    void onFocus(EventContext &context) override {
        CaretManager *caret = context.getCaretManager();
        float horz = (float) caret->data().getOffset().x;
        for_context(ctx, context) {
            GRect rect = ctx.rect();
            if (horz > rect.left() && horz < rect.right()) {
                ctx.focus();
                return;
            }
        }
        context.enter().focus();
    }
    void onEnter(EventContext &context, EventContext &enter, int idx) override {
        while (idx < 0) idx += size();
        enter.index = idx;
        enter.element = get(idx);
    }
    void onRelayout(EventContext &context, LayoutManager *sender) override {
        Offset offset;
        int height = 0;
        for_context(ctx, context) {
            height = (std::max)(height, ctx.logicHeight());
        }
        for_context(ctx, context) {
            ctx.current()->setLogicOffset(offset);
            offset.x += ctx.logicWidth();
            ctx.setLogicHeight(height);
        }
        m_width = offset.x;
        m_height = height;
    }
    void onFinishReflow(EventContext &context, Offset &offset, LayoutContext &layout) override {
        Element::onFinishReflow(context, offset, layout);
    }
    void onSelectionDelete(EventContext &context, SelectionState state) override {
        if (state == SelectionSelf) {
            if (context.selectedCount() == 1) {
                // 在单元格内选择
                Element::onSelectionDelete(context, state);
                return;
            }
        }
        if (context.tag().contain(TAG_UNDELETEABLE)) {
            return;
        }
        if (state == SelectionEnd || state == SelectionSelf) {
            if (auto *next = context.findNext(TAG_FOCUS)) {
                next->focus(false);
            }
        }
        if (m_prev) {
            EventContext prev = context.nearby(-1);
            if (prev.tag().contain(TAG_HEADER)) {
                if (m_next == nullptr || context.nearby(1).tag().contain(TAG_HEADER)) {
                    prev.remove();
                }
            }
        }
        context.remove();
    }
    void onClone(EventContext &context) override {
        context.insert(new FastRow(size()));
        context.nearby(1).focus();
    }
    void onUndo(Command command) override {
        Element::onUndo(command);
        if (command.type == CommandType::DeleteElement) {
            command.context->outer->update();
        }
    }
    void onNotify(EventContext &context, int type, NotifyParam param, NotifyValue other) override {
        context.outer->notify(type, param, other);
    }
    EventContext onDispatch(EventContext &context, int x, int y) override {
        auto horz = (float) x;
        for_context(ctx, context) {
            auto rect = ctx.rect();
            if (horz >= rect.left() && horz < rect.right()) {
                return ctx;
            }
            if (horz < rect.left()) {
                return ctx;
            }
        }
        return context.enter(-1);
    }

};
template <class RowElement = FastRow>
class FastTable : public Container<DisplayTable> {
public:
    //using RowElement = FastRow;
    typedef GColor (WINAPI *ColorProvider)(int type, int row, int col);
    int m_delta = 0;
    int m_top = 5;
    int m_column = 0;
    ColorProvider m_provider = nullptr;
    explicit FastTable(int column) : m_column(column) {}
    explicit FastTable(int row, int column) {
        m_column = column;
        for (int i = 0; i < row; ++i) {
            Container::append(new RowElement(column));
        }
    }
    RowElement *addRow(int column) {
        if (column > m_column) {
            m_column = column;
        }
        auto *row = new RowElement(column);
        append(row);
        return row;
    }
    RowElement *getRow(int line) { return (RowElement *) get(line); }
    template <typename Type = TextElement>
    Type *getItem(int row, int col) { return (Type *) ((RowElement *) get(row))->get(col); }
    virtual GColor getBackgroundColor(int row, int col) {
        if (m_provider) {
            return m_provider(0, row, col);
        }
        return SK_ColorTRANSPARENT;
    }
    virtual GColor getForegroundColor(int row, int col) {
        if (m_provider) {
            return m_provider(1, row, col);
        }
        return SK_ColorBLACK;
    }
public:
    Tag getTag(EventContext &context) override { return TAG("Table"); }
    int getLogicWidth(EventContext &context) override { return m_width + m_delta; }
    int getMinWidth(EventContext &context) override { return m_width; }
    void setLogicWidth(EventContext &context, int width) override {
        m_delta = width - m_width;
        for_context(row, context) {
            EventContext end = row.enter(-1);
            end.setLogicWidth(end.logicWidth() + m_delta);
            row.setLogicWidth(width);
        }
    }
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
            }
        }
        if (context.canEnter()) {
            sender->reflow(context.enter(), true);
        } else {
            m_height = 0;
        }
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
using TableElement = FastTable<>;
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
    Tag getTag(EventContext &context) override { return TAG("CodeBlock"); }
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
            GPath sym = DrawUtil::star(5, 5, 2);
            sym.offset(-8, 10);
            canvas->drawPath(sym, paint);
//            canvas->drawCircle(-5, 15, 5.5, paint);

        }
    }
    void onSelectionDelete(EventContext &context, SelectionState state) override {
        Element::onSelectionDelete(context, state);
        if (!context.canEnter()) {
            context.remove();
        }
    }
    void onUndo(Command command) override {
        if (command.type == CommandType::SeparateElement) {
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
    Tag getTag(EventContext &context) override { return {TAG("Switch Condition CodeBlock")}; }
    CodeBlockElement *addBlock(int num) { return (CodeBlockElement *) append(new CodeBlockElement(num)); }
private:
    int getWidth(EventContext &context) override { return Element::getWidth(context); }
public:
    void onBlur(EventContext &context, EventContext *focus, bool force) override {
        if (context.doc->m_onBlur) {
            if (context.doc->m_onBlur(&context, (int) focus, (int) force)) {
                return;
            }
        }
        if (force) { return; }
        bool clear = true;
        int line = getLineNumber();
        for (int i = 0; i < line; ++i) {
            if (!context.getLineViewer(i).empty()) {
                clear = false;
            }
        }
        if (clear) {
            if (focus && focus->include(&context)) {
                if(auto *prev = context.findPrev(TAG_FOCUS)) {
                    prev->focus(false);
                    return;
                }
            }
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
                if (context.nearby(-1).tag().contain(TAG("Condition"))
                    || (context.parent().tag() == TAG("CodeBlock") && context.isHead())) {
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
            if (ctx.nearby(1).enter().tag().contain(TAG("CodeBlock"))) {
                underlineRight += 25;
            }
            canvas->drawLine(lineLeft, lineTop, lineRight, lineTop, paint); // 上横线
            canvas->drawLine(lineLeft, lineTop, lineLeft, lineBottom, paint); // 竖线
            canvas->drawLine(lineLeft, lineBottom, underlineRight, lineBottom, paint); // 下横线
            canvas->drawLine(runawayLeft, ctx.height() - 10, lineRight, ctx.height() - 10, paint);
            // 逃逸线横线
            GPath path = DrawUtil::triangleRight(length, underlineRight, lineBottom);
            canvas->drawPath(path, paint);// 下边线三角形
        }
        // 需要绘制逃逸线
        Canvas canvas = context.getCanvas();
        if (context.nearby(1).tag().contain(TAG("CodeBlock"))) {
            int runawayBottom = context.height() + 12;
            canvas->drawLine(runawayLeft, runawayTop, runawayLeft, runawayBottom, paint); // 逃逸线竖线
            canvas->drawLine(runawayLeft, runawayBottom, runawayRight, runawayBottom, paint); // 逃逸线下横线
            GPath path = DrawUtil::triangleRight(length, runawayRight, runawayBottom); // 右三角
            canvas->drawPath(path, paint);
        } else {
            int runawayBottom = context.height() - 2;
            canvas->drawLine(runawayLeft, runawayTop, runawayLeft, context.height(), paint); // 逃逸线竖线
            // 逃逸线向下
            GPath path = DrawUtil::triangleDown(length, runawayLeft, runawayBottom);
            // 有可能后面还有流程语句 需要连接
            canvas->drawPath(path, paint);
        }

    }
    void onUndo(Command command) override {
        Element::onUndo(command);
    }
    void onSelectionDelete(EventContext& context, SelectionState state) override {
        Element::onSelectionDelete(context, state);
        if (!context.canEnter()) {
            context.remove();
        }
    }
};
class SingleBlockElement : public CodeBlockElement {
public:
    using CodeBlockElement::CodeBlockElement;
    Tag getTag(EventContext &context) override { return TAG("Single Condition CodeBlock"); }
    void onBlur(EventContext &context, EventContext *focus, bool force) override {
        if (context.doc->m_onBlur) {
            if (context.doc->m_onBlur(&context, (int) focus, (int) force)) {
                return;
            }
        }
        if (force) { return; }
        bool clear = true;
        int line = getLineNumber();
        for (int i = 0; i < line; ++i) {
            if (!context.getLineViewer(i).empty()) {
                clear = false;
            }
        }
        if (clear) {
            if (focus && focus->include(&context)) {
                context.findPrev(TAG_FOCUS)->focus(false);
                return;
            }
            context.replace(new AutoLineElement());
            context.outer->update();
            context.focus();
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
        if ((context.parent().tag() == TAG("CodeBlock") && context.isHead()) ||
            context.nearby(-1).tag().contain(TAG("Condition"))) {
            lineTop = 20;
        }
        canvas->drawLine(lineLeft, lineTop, lineRight, lineTop, paint);
        if (context.nearby(1).tag().contain(TAG("CodeBlock"))) {
            int lineBottom = context.height() + 12;
            canvas->drawLine(lineLeft, lineTop, lineLeft, lineBottom, paint); // 竖线
            canvas->drawLine(lineLeft, lineBottom, lineRight, lineBottom, paint); // 下边线
            GPath path = DrawUtil::triangleRight(5.7, lineRight, lineBottom);
            canvas->drawPath(path, paint);
        } else {
            int lineBottom = context.height() - 2;
            canvas->drawLine(lineLeft, lineTop, lineLeft, lineBottom, paint); // 竖线
            GPath path = DrawUtil::triangleDown(5.7, lineLeft, lineBottom);
            canvas->drawPath(path, paint);
        }
    }

    void onUndo(Command command) override {
        if (command.type == CommandType::AddElement) {
            EventContext *focus = command.context->findInnerLast(TAG_FOCUS);
            if (focus) {
                focus->setPos(command.pos);
                focus->focus(false);
            }
        }
        CodeBlockElement::onUndo(command);
    }
};
class LoopBlockElement : public SingleBlockElement {
public:
    using SingleBlockElement::SingleBlockElement;
    Tag getTag(EventContext &context) override { return TAG("Loop CodeBlock"); }
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
        if ((context.parent().tag() == TAG("CodeBlock") && context.isHead()) ||
            context.nearby(-1).tag().contain(TAG("Condition"))) {
            lineTop = 22;
        }
        int lineBottom = context.height() - 10;
        canvas->drawLine(lineLeft, lineTop, lineRight, lineTop, paint); // 上横线
        canvas->drawLine(lineLeft, lineTop, lineLeft, lineBottom, paint); // 竖线
        canvas->drawLine(lineLeft, lineBottom, lineRight, lineBottom, paint); // 下横线
        GPath path = DrawUtil::triangleRight(5.7, lineRight, lineTop);
        canvas->drawPath(path, paint);
    }
};
class SubElement : public Container<> {
    enum TableType {
        TableTypeNone,
        TableTypeHeader,
        TableTypeLocal,
    };
    class Table : public TableElement {
        class ParamRow : public FastRow {
        public:
            using FastRow::FastRow;
            void onClone(EventContext &context) override {
                if (m_undeleteable || m_header) {
                    auto *table = context.outer->cast<Table>();
                    if (context.outer->count() == 2) {
                        auto *row = new ParamRow(6);
                        row->setHeader(true);
                        row->getColumn(0)->setContent(_GT("Name"));
                        row->getColumn(1)->setContent(_GT("Type"));
                        row->getColumn(2)->setContent(_GT("Nullable"));
                        table->append(row);
                    }
                    table->append(new ParamRow(6));
                    context.outer->enter(-1).focus();
                    return;
                }
                context.insert(new ParamRow(size()));
                context.nearby(1).focus();
            }
        };
    public:
        TableType m_type = TableTypeNone;
        Table(int row, int column, TableType type) : FastTable(column), m_type(type) {
            for (int i = 0; i < row; ++i) {
                FastTable::append(new ParamRow(column));
            }
        }
        TableType type() { return m_type; }
        GColor getBackgroundColor(int row, int col) override {
            if (m_provider) {
                return m_provider(m_type << 1, row, col);
            }
            if (m_type == TableTypeHeader) {
                if (row == 0 || row == 2) {
                    return SkColorSetRGB(230, 237, 228);
                }
            }
            if (m_type == TableTypeLocal && row == 0) {
                //return SkColorSetRGB(217, 227, 240);
                return SkColorSetRGB(217, 227, 255);
            }
            return FastTable::getBackgroundColor(row, col);
        }
        GColor getForegroundColor(int row, int col) override {
            if (m_provider) {
                return m_provider((m_type << 1) + 1, row, col);
            }
            if (m_type == TableTypeHeader) {
                if (row == 0 || row == 2) {
                    return SK_ColorBLACK;
                }
                if (row == 1 && col == 0) {
                    return SkColorSetRGB(0, 0, 139); // Function Name
                }
                if (row == 1 && col == 3) {
                    return SkColorSetRGB(34, 139, 34); // Comment
                }
                if (col == 1) {
                    return SK_ColorBLUE;
                }
            }
            if (m_type == TableTypeLocal) {
                if (row == 0) {
                    return SK_ColorBLACK;
                }
                if (col == 1) {
                    return SK_ColorBLUE;
                }
            }
            return FastTable::getForegroundColor(row, col);
        }
        Tag getTag(EventContext &context) override {
            if (m_type == TableTypeHeader) {
                return FastTable::getTag(context).append(TAG(" SubHeader"));
            } else if (m_type == TableTypeLocal) {
                return FastTable::getTag(context).append(TAG(" LocalHeader"));
            } else {
                return FastTable::getTag(context);
            }
        }
    };
    Table *header = nullptr;
    Table *locals = nullptr;
public:
    SubElement() {
        header = new Table(2, 4, TableTypeHeader);
        if (auto *row = header->getRow(0)) {
            row->setHeader(true);
            row->getColumn(0)->m_data.append(_GT("Function Name"));
            row->getColumn(1)->m_data.append(_GT("Type"));
            row->getColumn(2)->m_data.append(_GT("Public"));
            row->getColumn(3)->m_data.append(_GT("Comment  "));
        }
        if (auto *row = header->getRow(1)) {
            row->setUndeleteable(true);
        }
        Container::append(header);
        locals = new Table(0, 4, TableTypeLocal);
        Container::append(locals);
    }
    GString &content(int col) { return header->getItem(1, col)->m_data; }
    void setContent(int row, int col, const GChar *value) { header->getItem(row, col)->m_data.assign(value); }
    FastRow *addParam(const GChar *name, const GChar *type) {
        if (header->getChildCount() == 2) {
            auto *param_header = header->addRow(6);
            param_header->setHeader(true);
            param_header->getColumn(0)->setContent(_GT("Name"));
            param_header->getColumn(1)->setContent(_GT("Type"));
            param_header->getColumn(2)->setContent(_GT("Nullable"));
        }
        auto *row = header->addRow(6);
        row->getColumn(0)->setContent(name);
        row->getColumn(1)->setContent(type);
        return row;
    }
    FastRow *addLocal(const GChar *name, const GChar *type) {
        if (locals->getChildCount() == 0) {
            auto *local_header = locals->addRow(4);
            local_header->setHeader(true);
            local_header->getColumn(0)->setContent(_GT("Name   "));
            local_header->getColumn(1)->setContent(_GT("Type   "));
        }
        auto *row = locals->addRow(4);
        row->getColumn(0)->setContent(name);
        row->getColumn(1)->setContent(type);
        return row;
    }
public:
    Tag getTag(EventContext &context) override { return {TAG("SubElement")}; }
    int getWidth(EventContext &context) override { return Element::getWidth(context); }
    void onSelectionDelete(EventContext &context, SelectionState state) override {
        for_context(ctx, context) {
            SelectionState selection = ctx.getSelectionState();
            if (selection != SelectionNone) {
                ctx.current()->onSelectionDelete(ctx, selection);
                if (ctx.tag().contain(TAG_TABLE)) {
                    auto type = ctx.cast<Table>()->type();
                    if (type == TableTypeHeader) {
                        if (ctx.enter().getSelectionState() != SelectionNone) {
                            EventContextRef ref(context.findInnerFirst(TAG_LINE));
                            EventContextRef last(context.findInnerLast(TAG_LINE));
                            while (ref) {
                                ref->getLineViewer().clear();
                                if (ref->compare(last)) {
                                    break;
                                }
                                ref = ref->findNext(TAG_LINE);
                            }
                            context.remove();
                            return;
                        }
                    }
                }
            }
        }
        context.relayout();
    }
};
class MultiLine : public RelativeElement {
private:
    class SingleLine : public SyntaxLineElement {
    public:
        int count = 6;
        Tag getTag(EventContext &context) override { return TAG("SingleLine Focus"); }
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
    bool expand = true;
public:
    Element *getHead() override {if (!expand) return nullptr; return &m_lines; }
    Element *getTail() override {if (!expand) return nullptr; return &m_lines; }
    Display getDisplay(EventContext &context) override { return DisplayBlock; }
    int getLogicHeight(EventContext &context) override { return m_lines.count * 25; }
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
            canvas->drawPath(DrawUtil::triangleDown(8, 9, 8), border);
            Root::onRedraw(context);
        } else {
            canvas->drawPath(DrawUtil::triangleRight(8, 7, 10), border);
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
    ModuleElement() = default;
    GString name;
    bool expand = false;
    bool layouted = false;
public:
    Tag getTag(EventContext &context) override { return TAG("Module"); }
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
            if (expand) {
                context.findInnerFirst(TAG_FOCUS)->focus(false);
            } else {
                context.focus();
            }
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
            canvas->drawPath(DrawUtil::triangleDown(8, 9, 8), border);
            Root::onRedraw(context);
        } else {
            canvas->drawPath(DrawUtil::triangleRight(8, 7, 10), border);
        }
    }
    void onEnterReflow(EventContext &context, Offset &offset) override {
        offset.y += 10;
    }
    bool onCanEnter(EventContext &context) override {
        return expand;
    }
};
class ClassElement : public Container<> {
    class ClassHeader : public TableElement {
        class ClassRow : public FastRow {
        public:
            using FastRow::FastRow;
            void onClone(EventContext &context) override {
                if (m_undeleteable || m_header) {
                    auto *table = context.outer->cast<ClassHeader>();
                    if (context.outer->count() == 2) {
                        auto *row = new ClassRow(4);
                        row->setHeader(true);
                        row->getColumn(0)->setContent(_GT("Field"));
                        row->getColumn(1)->setContent(_GT("Type"));
                        row->getColumn(2)->setContent(_GT("Array"));
                        row->getColumn(3)->setContent(_GT("Comment"));
                        table->append(row);
                    }
                    table->append(new ClassRow(4));
                    context.outer->enter(-1).focus();
                    return;
                }
                context.insert(new ClassRow(size()));
                context.nearby(1).focus();
            }
        };
    public:
        ClassHeader() : FastTable(4) {
            auto *row = new ClassRow(4);
            row->setHeader(true);
            row->getColumn(0)->setContent(_GT("Class Name"));
            row->getColumn(1)->setContent(_GT("Base Class"));
            row->getColumn(2)->setContent(_GT("Public"));
            row->getColumn(3)->setContent(_GT("Comment    "));
            FastTable::append(row);
            row = new ClassRow(4);
            row->setUndeleteable(true);
            FastTable::append(row);
        }
        GColor getBackgroundColor(int row, int col) override {
            if (m_provider) {
                return m_provider(3 << 1, row, col);
            }
            if (row == 0 || row == 2)
                return SK_ColorLTGRAY;
            return FastTable::getBackgroundColor(row, col);
        }
        GColor getForegroundColor(int row, int col) override {
            if (m_provider) {
                return m_provider((3 << 1) + 1, row, col);
            }
            if (row == 0 || row == 2) {
                return SK_ColorBLACK;
            }
            if (row == 1 && col == 0) {
                return SkColorSetRGB(0, 0, 139);
            }
            if (col == 1) {
                return SK_ColorBLUE;
            }
            if (col == 3) {
                return SkColorSetRGB(34, 139, 34);
            }
            return SK_ColorBLACK;
        }
        Tag getTag(EventContext &context) override {
            return FastTable::getTag(context).append(TAG(" ClassHeader"));
        }
    };
public:
    ClassElement() {
        Container::append(new ClassHeader());
    }
    int getWidth(EventContext &context) override { return Element::getWidth(context); }
    int getHeight(EventContext &context) override { return Container<>::getHeight(context); }
    void onSelectionDelete(EventContext &context, SelectionState state) override {
        for_context(ctx, context) {
            SelectionState selection = ctx.getSelectionState();
            if (selection != SelectionNone) {
                ctx.current()->onSelectionDelete(ctx, selection);
                if (selection == SelectionSelf || selection == SelectionEnd) {
                    goto leave;
                }
            }
        }
        //context.relayout();
        leave:
        context.enter().reflow();
    }

};
#endif //GEDITOR_TABLE_H
