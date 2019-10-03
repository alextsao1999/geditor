//
// Created by Alex on 2019/8/4.
//

#ifndef GEDITOR_TABLE_H
#define GEDITOR_TABLE_H

#include <SkBlurDrawLooper.h>
#include <SkColorFilter.h>
#include <SkMaskFilter.h>
#include <SkBlurMaskFilter.h>
#include <SkImageFilter.h>
#include <SkTypeface.h>
#include <SkBlurImageFilter.h>
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
    SkPaint paint;
    int m_column = 0;
    int m_min = 50;
public:
    int m_width = 0;
    int m_height = 25;
    explicit TextElement(int column) : m_column(column) {
        //paint.setTypeface(SkTypeface::CreateFromName("DengXian", SkTypeface::Style::kNormal));
        paint.setTextSize(14);
        paint.setTextEncoding(SkPaint::TextEncoding::kUTF16_TextEncoding);
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
    }
    int getLogicHeight(EventContext &context) override {
        return m_height;
    }
    int getLogicWidth(EventContext &context) override {
        int width = paint.measureText((const char*)m_data.c_str(), m_data.length() * 2) + 8;
        return width > m_min ? width : m_min;
    }
    void setLogicWidth(int width) override { m_width = width; }
    void setLogicHeight(int height) override { m_height = height; }
    int getWidth(EventContext& context) override {
        if (m_width) { return m_width; }
        return getLogicWidth(context);
    }
    Display getDisplay() override {
        return Display::Inline;
    }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        SkPaint border;
        border.setStyle(SkPaint::Style::kStroke_Style);
        border.setColor(SK_ColorLTGRAY);
        canvas->drawRect(canvas.size(), border);

        canvas->translate(0, paint.getTextSize());
        canvas->drawText((const char *) m_data.c_str(), m_data.length() * 2, 4, 2, paint);
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        TextCaretService service(Offset(4, 4), &context);
        service.moveTo(context.relative(x, y));
        service.commit((const char*)m_data.c_str(), m_data.length() * 2, paint);
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
        service.commit((const char*)m_data.c_str(), m_data.length() * 2, paint);
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
        service.commit((const char *) m_data.c_str(), m_data.length() * 2, paint);
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
    int getLogicHeight(EventContext &context) override {
        return m_height;
    }
    int getLogicWidth(EventContext &context) override {
        auto line = context.getLineViewer(m_column);
        int width = context.getRenderManager()->getTextMetrics().measure(line.str(), line.length()) + 8;

        return width > m_min ? width : m_min;
    }
    int getWidth(EventContext& context) override {
        if (m_width) { return m_width; }
        return getLogicWidth(context);
    }
    void setLogicWidth(int width) override { m_width = width; }
    void setLogicHeight(int height) override { m_height = height; }

    Display getDisplay() override {
        return Display::Inline;
    }
    void onRedraw(EventContext &context) override {
        Painter painter = context.getPainter();
        LineViewer line = context.getLineViewer(m_column);
        painter.drawRect(0, 0, getWidth(context), getHeight(context));
        painter.drawText(4, 4, line.str(), line.length());
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        Offset textOffset = context.relative(x, y) - Offset(4, 4);
        auto line = context.getLineViewer(m_column);
        auto meter = context.getRenderManager()->getTextMetrics();
        auto caret = context.getCaretManager();
        caret->data()->index = meter.getTextIndex(line.str(), line.length(), textOffset.x);
        caret->set(4 + textOffset.x, 4);
        caret->show();
    }
    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        auto ctx = caret->getEventContext();
        auto meter = context.getRenderManager()->getTextMetrics();
        LineViewer line;
        if (code == VK_RIGHT || code == VK_LEFT) {
            line = ctx->getLineViewer(m_column);
            if (code == VK_RIGHT) {
                caret->data()->index++;
                if (caret->data()->index > line.length()) {
                    if (!context.next()) {
                        context.prev();
                        caret->data()->index = line.length();
                        return;
                    }
                    caret->data()->index = 0;
                }
            }
            if (code == VK_LEFT) {
                caret->data()->index--;
                if (caret->data()->index < 0) {
                    caret->data()->index = 0;
                }
            }
        }
        if (code == VK_UP || code == VK_DOWN) {
        }
        if (line.empty())
            return;
        int x = meter.measure(line.str(), caret->data()->index);
        caret->set(x + 4, 4);
        caret->show();
    };
    void onInputChar(EventContext &context, int ch) override {
        auto caret = context.getCaretManager();
        auto line = context.getLineViewer(m_column);
        switch (ch) {
            case VK_BACK:
                if (caret->data()->index > 0) {
                    int index = --caret->data()->index;
                    if (index >= 0) {
                        line.remove(index);
                    }
                }
                break;
            case VK_RETURN:
                m_height += 10;
                context.start().reflow();
                break;
            default:
                context.push(CommandType::Add, CommandData(caret->data()->index, ch));
                line.insert(caret->data()->index++, ch);
                context.reflowBrother();
                break;
        }
        if (context.outer) {
            context.outer->notify(WidthChange, 0, m_column);
        }

        context.redraw();
        caret->autoSet(4, 4, m_column);
    }

};
class ButtonElement : public RelativeElement {
public:
    GString m_data;
    Display getDisplay() override {
        return Display::Block;
    }
    int getLogicHeight(EventContext &context) override {
        return 50;
    }
    int getWidth(EventContext &context) override {
        return 50;
    }

    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        SkPaint line;
        line.setColor(SK_ColorBLUE);
        for (int i = 0; i < 30; ++i) {
            canvas->drawLine(10, 4, 10 + i * 4, 4, line);
        }
        SkPaint style;
        style.setColor(SK_ColorBLACK);
        style.setTextSize(15);
        style.setTextEncoding(SkPaint::kUTF16_TextEncoding);
        style.setColor(SK_ColorRED);
        canvas->drawText(m_data.c_str(), m_data.size() * 2, 4, 4 + 15, style);

        SkRect rect{};
        rect.setXYWH(0, 0, getWidth(context), getHeight(context));

        SkPath path;
        float radiusArray[8];
        for (auto &arr : radiusArray) {
            arr = 6.0f;
        }
        SkPaint paint;
        path.addRoundRect(rect, radiusArray);
        paint.setColor(SK_ColorRED);
        paint.setStrokeWidth(1);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(true);
        canvas->drawPath(path, paint);


    }

    void onMouseEnter(EventContext &context, int x, int y) override {

    }

    void onMouseLeave(int x, int y) override {

    }
};
class LineElement : public RelativeElement {
public:
    SkPaint paint;
    SkPaint style;
    int i = 0;
    LineElement() {
        paint.setTypeface(SkTypeface::CreateFromName("Monoca", SkTypeface::Style::kNormal));
        paint.setTextSize(15);
        paint.setTextEncoding(SkPaint::TextEncoding::kUTF16_TextEncoding);
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        //paint.setLooper(SkBlurDrawLooper::Create(SK_ColorGRAY, 20, 4, 4));

    }
    int getLogicHeight(EventContext &context) override {
        return 25;
    }
    Display getDisplay() override {
        return Display::Line;
    }
    Element *copy() override {
        return new LineElement();
    }
    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas(&style);
        SkPaint border;
        border.setStyle(SkPaint::Style::kStroke_Style);
        border.setColor(SK_ColorLTGRAY);
        canvas->drawRect(canvas.size(), border);
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
                SkColorFilter::CreateModeFilter(SkColorSetARGB(255, 255, 250, 227), SkXfermode::Mode::kDarken_Mode));

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
        if (type == WidthChange || type == SizeChange) {
            if (context.outer) {
                context.outer->notify(WidthChange, 0, p2);
            }
        }
    }
};
class InlineTableElement : public Container {
public:
    InlineTableElement(int line, int column) : Container(Display::Inline) {
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
    explicit RowElement(int column) : Container(Display::Line) {
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
    int getColumnWidth(EventContext &context, int column) {
        return m_index.at(column)->getLogicWidth(context);
    }
    void setColumnWidth(int column, int width) {
        m_index.at(column)->setLogicWidth(width);
    }
    void onNotify(EventContext &context, int type, int p1, int p2) override {
        if (type == WidthChange || type == SizeChange) {
            if (context.outer) {
                context.outer->notify(WidthChange, 0, p2);
            }
        }
    }
};
class TableElement : public Container {
public:
    bool m_init = false;
    TableElement(int line, int column) {
        for (int i = 0; i < line; ++i) {
            Element *element = new RowElement(column);
            append(element);
        }
    }
    void replace(int line, int column, Element *element) {
        auto *row = (RowElement *) m_index.at(line);
        auto *old = row->m_index.at(column);
        row->m_index.at(column) = element;
        delete old;
    }
    void setColumnWidth(int column, int width) {
        for (auto *ele : m_index) {
            auto *row = (RowElement *) ele;
            row->setColumnWidth(column, width);
        }
    }
    void onNotify(EventContext &ctx, int type, int p1, int p2) override {
        EventContext context = ctx.enter();
        int width = 0;
        while (context.has()) {
            auto *element = (RowElement *) context.current();
            int cur_width = element->getColumnWidth(context, p2);
            if (cur_width > width)
                width = cur_width;
            context.next();
        }
        setColumnWidth(p2, width);
        context.getLayoutManager()->reflowEnter(context.start());
        ctx.redraw();
    }
    void onLeaveReflow(EventContext &context) override {
        if (m_init) {
            return;
        }
        m_init = true;
        int i = 0;
        int colMaxWidth[63] = {0};
        // 排列完成之后更新高度
        EventContext row = context.enter();
        while (row.has()) {
            EventContext col = row.enter();
            i = 0;
            while (col.has()) {
                int width = CallEvent(col, getWidth);
                if (width > colMaxWidth[i]) {
                    colMaxWidth[i] = width;
                }
                i++;
                col.next();
            }
            row.next();
        }
        for (int j = 0; j < i; ++j) {
            setColumnWidth(j, colMaxWidth[j]);
        }
        context.getLayoutManager()->reflowEnter(row.start());

    }

};

#endif //GEDITOR_TABLE_H
