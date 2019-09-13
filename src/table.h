//
// Created by Alex on 2019/8/4.
//

#ifndef GEDITOR_TABLE_H
#define GEDITOR_TABLE_H

#include "document.h"

class LineElement : public RelativeElement {
    using RelativeElement::RelativeElement;
public:
    int getLogicHeight(EventContext &context) override {
        return 23;
    }
    int getLogicWidth(EventContext &context) override {
        auto &str = context.getLineViewer().content();
        return context.getPaintManager()->getTextMeter().meterWidth(str.c_str(), str.size()) + 8;
    }
    Display getDisplay() override {
        return Display::Line;
    }
    Element *copy() override {
        return new LineElement();
    }
    void redraw(EventContext &context) override {
        Painter painter = context.getPainter();
        LineViewer line = context.getLineViewer();
        painter.setTextColor(RGB(0, 0, 0));
        auto str = &line.content();
        if (str != nullptr) {
            painter.drawText(4, 4, str->c_str(), str->size());
        }
        //painter.drawRect(0, 0, getWidth(context), getHeight(context));
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        Offset textOffset = getRelOffset(context, x, y) - Offset(4, 4);
        auto line = context.getLineViewer();
        auto meter = context.getPaintManager()->getTextMeter();
        auto caret = context.getCaretManager();
        caret->data()->index = meter.getTextIndex(line.content().c_str(), line.content().size(), textOffset.x);
        caret->set(textOffset.x + 4, 4);
        caret->show();
        context.reflow();
        context.getPaintManager()->refresh();
    }
    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        auto ctx = caret->getEventContext();
        auto meter = context.getPaintManager()->getTextMeter();
        LineViewer line;
        if (code == VK_RIGHT || code == VK_LEFT) {
            line = ctx->getLineViewer();
            if (code == VK_RIGHT) {
                caret->data()->index++;
                if (caret->data()->index > line.content().size()) {
                    if (!context.next()) {
                        context.prev();
                        caret->data()->index = line.content().size();
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
            int width = meter.meterWidth(ctx->getLineViewer().content().c_str(), caret->data()->index);
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
            caret->data()->index = meter.getTextIndex(line.content().c_str(), line.content().size(), width);
        }
        if (line.empty())
            return;
        int x = meter.meterWidth(line.content().c_str(), caret->data()->index);
        caret->set(x + 4, 4);
        caret->show();
    };
    void onInputChar(EventContext &context, int ch) override {
        auto caret = context.getCaretManager();
        auto &str = context.getLineViewer().content();
        switch (ch) {
            case VK_BACK:
                if (caret->data()->index > 0) {
                    int index = --caret->data()->index;
                    if (index >= 0) {
                        str.erase(str.begin() + index);
                    }
                } else {
                    if (!context.prev()) {
                        return;
                    }
                    caret->data()->index = context.getLineViewer().content().size();
                    context.combine();
                    context.reflow();
                }
                break;
            case VK_RETURN:
            {
                context.copyLine();
                context.reflow();
                int idx = caret->data()->index;
                auto &last = context.getLineViewer().content();
                auto temp = last.substr((unsigned) idx, last.size());
                last.erase((unsigned) idx, last.size());
                context.next();
                context.getLineViewer().content() = temp;
                caret->data()->index = 0;
            }
                break;
            default:
                context.push(CommandType::Add, CommandData(caret->data()->index, ch));
                str.insert(str.begin() + caret->data()->index++, (GChar) ch);
                break;
        }
        context.redraw();
        caret->autoSet(4, 4);
    }
};

class ColumnElement : public RelativeElement {
private:
    int m_column = 0;
    int m_min = 50;
public:
    int m_width = 0;
    explicit ColumnElement(int column) : m_column(column) {}
    int getLogicHeight(EventContext &context) override {
        return 25;
    }
    int getLogicWidth(EventContext &context) override {
        auto &str = context.getLineViewer().content(m_column);
        int width = context.getPaintManager()->getTextMeter().meterWidth(str.c_str(), str.size()) + 8;
        return width > m_min ? width : m_min;
    }
    int getWidth(EventContext& context) override
    {
        if (m_width) { return m_width; }
        return getLogicWidth(context);
    }
    Display getDisplay() override {
        return Display::Inline;
    }
    void redraw(EventContext &context) override {
        Painter painter = context.getPainter();
        LineViewer line = context.getLineViewer();
        auto str = &line.content(m_column);
        painter.setTextColor(RGB(0, 0, 0));
        painter.drawRect(0, 0, getWidth(context), getHeight(context));
        if (str != nullptr) {
            painter.drawText(4, 4, str->c_str(), str->size());
        }
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        Offset textOffset = getRelOffset(context, x, y) - Offset(4, 4);
        auto line = context.getLineViewer();
        auto meter = context.getPaintManager()->getTextMeter();
        auto caret = context.getCaretManager();
        caret->data()->index = meter.getTextIndex(line.content(m_column).c_str(), line.content(m_column).size(), textOffset.x);
        caret->set(4 + textOffset.x, 4);
        caret->show();
        context.reflow();
        context.getPaintManager()->refresh();
    }
    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        auto ctx = caret->getEventContext();
        auto meter = context.getPaintManager()->getTextMeter();
        LineViewer line;
        if (code == VK_RIGHT || code == VK_LEFT) {
            line = ctx->getLineViewer();
            if (code == VK_RIGHT) {
                caret->data()->index++;
                if (caret->data()->index > line.content(m_column).size()) {
                    if (!context.next()) {
                        context.prev();
                        caret->data()->index = line.content(m_column).size();
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
        int x = meter.meterWidth(line.content(m_column).c_str(), caret->data()->index);
        caret->set(x + 4, 4);
        caret->show();
    };
    void onInputChar(EventContext &context, int ch) override {
        auto caret = context.getCaretManager();
        auto &str = context.getLineViewer().content(m_column);
        switch (ch) {
            case VK_BACK:
                if (caret->data()->index > 0) {
                    int index = --caret->data()->index;
                    if (index >= 0) {
                        str.erase(str.begin() + index);
                    }
                }
                break;
            case VK_RETURN:
                break;
            default:
                context.push(CommandType::Add, CommandData(caret->data()->index, ch));
                str.insert(str.begin() + caret->data()->index++, (GChar) ch);
                break;
        }
        if (context.outer) {
            context.outer->current()->onNotify(*context.outer, WidthChange, 0, m_column);
        }

        context.reflowBrother();
        context.redraw();
        caret->autoSet(4, 4, m_column);
    }

};
class TextElement : public RelativeElement {
private:
    GString m_data;
    int m_column = 0;
    int m_min = 50;
public:
    int m_width = 0;
    explicit TextElement(int column) : m_column(column) {}
    int getLogicHeight(EventContext &context) override {
        return 25;
    }
    int getLogicWidth(EventContext &context) override {
        int width = context.getPaintManager()->getTextMeter().meterWidth(m_data.c_str(), m_data.size()) + 8;
        return width > m_min ? width : m_min;
    }
    int getWidth(EventContext& context) override
    {
        if (m_width) { return m_width; }
        return getLogicWidth(context);
    }
    Display getDisplay() override {
        return Display::Inline;
    }
    void redraw(EventContext &context) override {
        Painter painter = context.getPainter();
        LineViewer line = context.getLineViewer();
        painter.setTextColor(RGB(0, 0, 0));
        painter.drawRect(0, 0, getWidth(context), getHeight(context));
        painter.drawText(4, 4, m_data.c_str(), m_data.size());
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        Offset textOffset = getRelOffset(context, x, y) - Offset(4, 4);
        auto line = context.getLineViewer();
        auto meter = context.getPaintManager()->getTextMeter();
        auto caret = context.getCaretManager();
        caret->data()->index = meter.getTextIndex(m_data.c_str(), m_data.size(), textOffset.x);
        caret->set(4 + textOffset.x, 4);
        caret->show();
        context.reflow();
        context.getPaintManager()->refresh();
    }
    void onKeyDown(EventContext &context, int code, int status) override {
        auto caret = context.getCaretManager();
        auto ctx = caret->getEventContext();
        auto meter = context.getPaintManager()->getTextMeter();
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

        int x = meter.meterWidth(m_data.c_str(), caret->data()->index);
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
            EventContext ctx = context.outer->enter();
            context.outer->current()->onNotify(*context.outer, Element::None, 0, m_column);
        }

        context.redraw();
        auto meter = context.getPaintManager()->getTextMeter();
        int x = meter.meterWidth(m_data.c_str(), caret->data()->index);
        caret->set(x + 4, 4);
        caret->show();

    }

};

class RowElement : public RelativeElement {
public:
    ElementIndex m_index;
    RowElement() = default;
    explicit RowElement(int column) {
        for (int i = 0; i < column; ++i) {
            Element *element = new ColumnElement(i);
            m_index.append(element);
        }
    }

    bool hasChild() override {
        return m_index.size() > 0;
    }

    int getLogicHeight(EventContext &context) override {
        return 25;
    }
    ElementIndex *children() override {
        return &m_index;
    }
    Display getDisplay() override {
        return Display::Line;
    }
    void append(Element *element) {
        m_index.append(element);
    }
    int getColumnWidth(EventContext &context, int column) {
        return m_index.at(column)->getLogicWidth(context);
    }
    int getColumnRealWidth(EventContext &context, int column) {
        return m_index.at(column)->getWidth(context);
    }
    void setColumnWidth(int column, int width)
    {
        ((ColumnElement*) m_index.at(column))->m_width = width;
    }
    void onNotify(EventContext &context, NotifyType type, int p1, int p2) override {
        if (type == WidthChange) {
            if (context.outer) {
                EventContext ctx = context.outer->enter();
                context.outer->current()->onNotify(ctx, WidthChange, 0, p2);
            }
        }
    }
};
class InlineRowElement : public RowElement {
public:
    explicit InlineRowElement(int line, int column) {
        for (int i = 0; i < column; ++i) {
            Element *element = new ColumnElement(i * line);
            m_index.append(element);
        }
    }
    Display getDisplay() override {
        return Display::Block;
    }
};

class TableElement : public RelativeElement {
public:
    ElementIndex m_index;
    TableElement(int line, int column, bool inLine = false) {
        for (int i = 0; i < line; ++i) {
            Element* element = inLine ? new InlineRowElement(i, column) : new RowElement(column);
            m_index.append(element);
        }
    }

    Element *replace(int line, int column, Element *element) {
        auto *row = (RowElement *) m_index.at(line);
        auto *old = row->m_index.at(column);
        row->m_index.at(column) = element;
        return old;
    }
    bool hasChild() override {
        return m_index.size() > 0;
    }

    int getLogicHeight(EventContext &context) override {
        return m_index.m_buffer.size() * 25;
    }
    ElementIndex *children() override {
        return &m_index;
    }
    Display getDisplay() override {
        return Display::Block;
    }
    void append(Element *element) {
        m_index.append(element);
    }

    void onNotify(EventContext &context, NotifyType type, int p1, int p2) override {
        int width = 0;
        while (context.has()) {
            auto *element = (RowElement *) context.current();
            int cur_width = element->getColumnWidth(context, p2);
            if (cur_width > width)
                width = cur_width;
            context.next();
        }
        context.init();
        while (context.has()) {
            auto *element = (RowElement *) context.current();
            element->setColumnWidth(p2, width);
            context.next();
        }
        context.init();
        context.doc->getContext()->m_layoutManager.reflowEnter(context);
    }
};

#endif //GEDITOR_TABLE_H
