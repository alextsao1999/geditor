//
// Created by Alex on 2019/8/4.
//

#ifndef GEDITOR_TABLE_H
#define GEDITOR_TABLE_H

#include "document.h"

class TextElement : public RelativeElement {
    using RelativeElement::RelativeElement;
public:
    int getLogicHeight(EventContext &context) override {
        return 23;
    }
    int getLogicWidth(EventContext &context) override {
        auto &str = context.getLineViewer().getContent();
        return context.getPaintManager()->getTextMeter().meterWidth(str.c_str(), str.length());
    }
    Display getDisplay() override {
        return Display::Line;
    }
    Element *copy() override {
        return new TextElement(m_parent);
    }
    void redraw(EventContext &context) override {
        Painter painter = context.getPainter();
        LineViewer line = context.getLineViewer();
        painter.setTextColor(RGB(0, 0, 0));
        painter.drawText(4, 4, line.getContent().c_str(), line.getContent().size());
        //painter.drawRect(2, 2, getWidth(context) + 6, getHeight(context));
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        Offset textOffset = getRelOffset(x, y) - Offset(4, 4);
        auto line = context.getLineViewer();
        auto meter = context.getPaintManager()->getTextMeter();
        auto caret = context.getCaretManager();
        caret->data()->index = meter.getTextIndex(line.getContent().c_str(), line.getContent().size(), textOffset.x);
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
                if (caret->data()->index > line.getContent().size())
                    caret->data()->index = line.getContent().size();
            }
            if (code == VK_LEFT) {
                caret->data()->index--;
                if (caret->data()->index < 0)
                    caret->data()->index = 0;
            }
        }
        if (code == VK_UP || code == VK_DOWN) {
            int width = meter.meterWidth(ctx->getLineViewer().getContent().c_str(), caret->data()->index);
            if (code == VK_UP){
                if (!ctx->prev())
                    return;
            } else {
                if (!ctx->next()) {
                    ctx->prev();
                    return;
                }
            }
            line = ctx->getLineViewer();
            caret->data()->index = meter.getTextIndex(line.getContent().c_str(), line.getContent().size(), width);
        }
        if (line.empty())
            return;
        int x = meter.meterWidth(line.getContent().c_str(), caret->data()->index);
        caret->set(x + 4, 4);
        caret->show();
    };
    void onInputChar(EventContext &context, int ch) override {
        auto caret = context.getCaretManager();
        auto &str = context.getLineViewer().getContent();
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
                    caret->data()->index = context.getLineViewer().getContent().size();
                    context.combine();
                    context.reflow();
                }
                break;
            case VK_RETURN:
            {
                context.copyLine();
                context.reflow();
                int idx = caret->data()->index;
                auto &last = context.getLineViewer().getContent();
                auto temp = last.substr((unsigned) idx, last.length());
                last.erase(last.begin() + idx, last.end());
                context.next();
                context.getLineViewer().getContent() = temp;
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

class ColumnTextElement : public RelativeElement {
private:
    int column = 0;
public:
    explicit ColumnTextElement(Root *parent, int column) : RelativeElement(parent), column(column) {}
    explicit ColumnTextElement(int column) : column(column) {}
    int getLogicHeight(EventContext &context) override {
        return 23;
    }
    int getLogicWidth(EventContext &context) override {
        auto &str = context.getLineViewer().getContent(column);
        int width = context.getPaintManager()->getTextMeter().meterWidth(str.c_str(), str.length());
        return width + 20;
    }
    Display getDisplay() override {
        return Display::Inline;
    }
    Element *copy() override {
        return nullptr;
    }
    void redraw(EventContext &context) override {
        Painter painter = context.getPainter();
        LineViewer line = context.getLineViewer();
        painter.setTextColor(RGB(0, 0, 0));
        painter.drawText(4, 4, line.getContent(column).c_str(), line.getContent(column).size());
        painter.drawRect(2, 2, getWidth(context) - 2, getHeight(context));
    }
    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        Offset textOffset = getRelOffset(x, y) - Offset(4, 4);
        auto line = context.getLineViewer();
        auto meter = context.getPaintManager()->getTextMeter();
        auto caret = context.getCaretManager();
        caret->data()->index = meter.getTextIndex(line.getContent(column).c_str(), line.getContent(column).size(), textOffset.x);
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
                if (caret->data()->index > line.getContent(column).size())
                    caret->data()->index = line.getContent(column).size();
            }
            if (code == VK_LEFT) {
                caret->data()->index--;
                if (caret->data()->index < 0)
                    caret->data()->index = 0;
            }
        }
        if (code == VK_UP || code == VK_DOWN) {

        }
        if (line.empty())
            return;
        int x = meter.meterWidth(line.getContent(column).c_str(), caret->data()->index);
        caret->set(x + 4, 4);
        caret->show();
    };
    void onInputChar(EventContext &context, int ch) override {
        auto caret = context.getCaretManager();
        auto &str = context.getLineViewer().getContent(column);
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
        context.reflowBrother();
        context.redraw();
        caret->autoSet(4, 4, column);
    }

};

class RowElement : public RelativeElement {
public:
    ElementIndex m_index;

    bool hasChild() override {
        return m_index.size() > 0;
    }
    int getLogicHeight(EventContext &context) override {
        return 23;
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

};


#endif //GEDITOR_TABLE_H
