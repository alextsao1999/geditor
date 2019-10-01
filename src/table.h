//
// Created by Alex on 2019/8/4.
//

#ifndef GEDITOR_TABLE_H
#define GEDITOR_TABLE_H

#include "document.h"
class LineElement : public RelativeElement {
public:
    int getLogicHeight(EventContext &context) override {
        return 23;
    }
    int getLogicWidth(EventContext &context) override {
        auto line = context.getLineViewer();
        return context.getRenderManager()->getTextMetrics().measure(line.str(), line.length()) + 8;
    }
    Display getDisplay() override {
        return Display::Line;
    }
    Element *copy() override {
        return new LineElement();
    }
    void onRedraw(EventContext &context) override {
        Painter painter = context.getPainter();
        LineViewer line = context.getLineViewer();
        painter.drawText(4, 4, line.str(), line.length());
        //painter.drawRect(0, 0, getWidth(context), getHeight(context));
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        Offset textOffset = context.relative(x, y) - Offset(4, 4);
        auto line = context.getLineViewer();
        auto meter = context.getRenderManager()->getTextMetrics();
        auto caret = context.getCaretManager();
        caret->data()->index = meter.getTextIndex(line.str(), line.length(), textOffset.x);
        caret->set(textOffset.x + 4, 4);
        caret->show();
    }
    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        auto ctx = caret->getEventContext();
        auto meter = context.getRenderManager()->getTextMetrics();
        LineViewer line;
        if (code == VK_RIGHT || code == VK_LEFT) {
            line = ctx->getLineViewer();
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
            int width = meter.measure(ctx->getLineViewer().str(), caret->data()->index);
            if (code == VK_UP){
                if (!ctx->prev())
                    return;
            } else {
                if (!ctx->next()) {
                    ctx->prev();
                    return;
                }
            }
            while (caret->enter()) {
                ctx = caret->getEventContext();
            }
            line = ctx->getLineViewer();
            caret->data()->index = meter.getTextIndex(line.str(), line.length(), width);
        }
        if (line.empty())
            return;
        int x = meter.measure(line.str(), caret->data()->index);
        caret->set(x + 4, 4);
        caret->show();
        context.redraw();
    };
    void onInputChar(EventContext &context, int ch) override {
        auto caret = context.getCaretManager();
        auto line = context.getLineViewer();
        switch (ch) {
            case VK_BACK:
                if (caret->data()->index > 0) {
                    int index = --caret->data()->index;
                    if (index >= 0) {
                        line.remove(index);
                    }
                } else {
                    if (!context.prev()) {
                        return;
                    }
                    caret->data()->index = context.getLineViewer().length();
                    context.combine();
                    context.reflow();
                }
                break;
            case VK_RETURN:
            {
                context.copyLine();
                context.reflow();
                context.next();
                int idx = caret->data()->index;
                context.getLineViewer().append(line.str() + idx, line.length() - idx);
                line.erase(idx, line.length() - idx);
                caret->data()->index = 0;
            }
                break;
            default:
                context.push(CommandType::Add, CommandData(caret->data()->index, ch));
                line.insert(caret->data()->index++, ch);
                break;
        }
        context.redraw();
        caret->autoSet(4, 4);
    }
    void onFocus(EventContext &context) override {
        context.redraw();
    }
    void onBlur(EventContext &context) override {
        context.redraw();
    }
};
class PosElement : public RelativeElement {
private:
    GString m_data;
    int m_min = 50;
public:
    int m_width = 0;
    int m_height = 25;
    explicit PosElement() {}
    int getLogicHeight(EventContext &context) override {
        return m_height;
    }
    int getLogicWidth(EventContext &context) override {
        int width = context.getRenderManager()->getTextMetrics().measure(m_data.c_str(), m_data.size()) + 8;
        return width > m_min ? width : m_min;
    }
    Display getDisplay() override {
        return Display::Block;
    }
    void onRedraw(EventContext &context) override {
        auto painter = context.getPainter();
        LineViewer line = context.getLineViewer();
        //painter.drawRect(0, 0, getWidth(context), getHeight(context));
        //paint.measureText(m_data.c_str(), m_data.size() * 2);
        painter.drawText(4, 4, m_data.c_str(), m_data.size());

        Canvas canvas = context.getCanvas();
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
            arr = 7.0f;
        }
        SkPaint paint;
        path.addRoundRect(rect, radiusArray);
        paint.setColor(SK_ColorRED);
        paint.setStrokeWidth(3);
        paint.setStyle(SkPaint::kStroke_Style);
        //paint.setAntiAlias(true);
        canvas->drawPath(path, paint);
        //canvas.drawText(4, 4, m_data.c_str(), m_data.size(), paint);
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        Offset textOffset = getRelOffset(context, x, y) - Offset(4, 4);
        auto line = context.getLineViewer();
        auto meter = context.getRenderManager()->getTextMetrics();
        auto caret = context.getCaretManager();
        caret->data()->index = meter.getTextIndex(m_data.c_str(), m_data.size(), textOffset.x);
        caret->set(4 + textOffset.x, 4);
        caret->show();
    }
    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        auto ctx = caret->getEventContext();
        auto meter = context.getRenderManager()->getTextMetrics();
        if (code == VK_RIGHT || code == VK_LEFT) {
            if (code == VK_RIGHT) {
                caret->data()->index++;
                if (caret->data()->index > m_data.size()) {
                    if (!context.next()) {
                        context.prev();
                        caret->data()->index = m_data.size();
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

        int x = meter.measure(m_data.c_str(), caret->data()->index);
        caret->set(x + 4, 4);
        caret->show();
    };
    void onInputChar(EventContext &context, int ch) override {
        auto caret = context.getCaretManager();
        switch (ch) {
            case VK_BACK:
                if (caret->data()->index > 0) {
                    int index = --caret->data()->index;
                    if (index >= 0) {
                        m_data.erase(m_data.begin() + index);
                    }
                }
                break;
            case VK_RETURN:
                break;
            default:
                context.push(CommandType::Add, CommandData(caret->data()->index, ch));
                m_data.insert(m_data.begin() + caret->data()->index++, (GChar) ch);
                break;
        }
        context.reflow();
        context.redraw();
        auto meter = context.getRenderManager()->getTextMetrics();
        int x = meter.measure(m_data.c_str(), caret->data()->index);
        caret->set(x + 4, 4);
        caret->show();

    }

    void onMouseMove(EventContext &context, int x, int y) override {
        Offset rel = getRelOffset(context, x, y);
        GChar str[255];
        wsprintf(str, _GT("your x: %d, y: %d\0"), rel.x, rel.y);
        m_data.clear();
        m_data.append(str);
        context.redraw();
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
    int getLogicHeight(EventContext &context) override {
        return m_height;
    }
    int getLogicWidth(EventContext &context) override {
        int width = context.getRenderManager()->getTextMetrics().measure(m_data.c_str(), m_data.size()) + 8;
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
        Painter painter = context.getPainter();
        LineViewer line = context.getLineViewer();
        painter.drawRect(0, 0, getWidth(context), getHeight(context));
        painter.drawText(4, 4, m_data.c_str(), m_data.size());

    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        Offset textOffset = getRelOffset(context, x, y) - Offset(4, 4);
        auto line = context.getLineViewer();
        auto meter = context.getRenderManager()->getTextMetrics();
        auto caret = context.getCaretManager();
        caret->data()->index = meter.getTextIndex(m_data.c_str(), m_data.size(), textOffset.x);
        caret->set(4 + textOffset.x, 4);
        caret->show();
    }
    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        auto ctx = caret->getEventContext();
        auto meter = context.getRenderManager()->getTextMetrics();
        if (code == VK_RIGHT || code == VK_LEFT) {
            if (code == VK_RIGHT) {
                caret->data()->index++;
                if (caret->data()->index > m_data.size()) {
                    if (!context.next()) {
                        context.prev();
                        caret->data()->index = m_data.size();
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

        int x = meter.measure(m_data.c_str(), caret->data()->index);
        caret->set(x + 4, 4);
        caret->show();
    };
    void onInputChar(EventContext &context, int ch) override {
        auto caret = context.getCaretManager();
        switch (ch) {
            case VK_BACK:
                if (caret->data()->index > 0) {
                    int index = --caret->data()->index;
                    if (index >= 0) {
                        m_data.erase(m_data.begin() + index);
                    }
                }
                break;
            case VK_RETURN:
                break;
            default:
                context.push(CommandType::Add, CommandData(caret->data()->index, ch));
                m_data.insert(m_data.begin() + caret->data()->index++, (GChar) ch);
                break;
        }
        if (context.outer) {
            context.outer->notify(WidthChange, 0, m_column);
        }
        context.reflow();
        context.redraw();
        auto meter = context.getRenderManager()->getTextMetrics();
        int x = meter.measure(m_data.c_str(), caret->data()->index);
        caret->set(x + 4, 4);
        caret->show();

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
        auto line = context.getLineViewer();
        int width = context.getRenderManager()->getTextMetrics().measure(line.str(m_column), line.length(m_column)) + 8;

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
        LineViewer line = context.getLineViewer();
        painter.drawRect(0, 0, getWidth(context), getHeight(context));
        painter.drawText(4, 4, line.str(m_column), line.length(m_column));
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        Offset textOffset = context.relative(x, y) - Offset(4, 4);
        auto line = context.getLineViewer();
        auto meter = context.getRenderManager()->getTextMetrics();
        auto caret = context.getCaretManager();
        caret->data()->index = meter.getTextIndex(line.str(m_column), line.length(m_column), textOffset.x);
        caret->set(4 + textOffset.x, 4);
        caret->show();
    }
    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        auto ctx = caret->getEventContext();
        auto meter = context.getRenderManager()->getTextMetrics();
        LineViewer line;
        if (code == VK_RIGHT || code == VK_LEFT) {
            line = ctx->getLineViewer();
            if (code == VK_RIGHT) {
                caret->data()->index++;
                if (caret->data()->index > line.length(m_column)) {
                    if (!context.next()) {
                        context.prev();
                        caret->data()->index = line.length(m_column);
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
        int x = meter.measure(line.str(m_column), caret->data()->index);
        caret->set(x + 4, 4);
        caret->show();
    };
    void onInputChar(EventContext &context, int ch) override {
        auto caret = context.getCaretManager();
        auto line = context.getLineViewer();
        switch (ch) {
            case VK_BACK:
                if (caret->data()->index > 0) {
                    int index = --caret->data()->index;
                    if (index >= 0) {
                        line.remove(index, m_column);
                    }
                }
                break;
            case VK_RETURN:
                m_height += 10;
                context.start().reflow();
                break;
            default:
                context.push(CommandType::Add, CommandData(caret->data()->index, ch));
                line.insert(caret->data()->index++, ch, m_column);
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
