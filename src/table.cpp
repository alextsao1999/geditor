//
// Created by Alex on 2019/8/4.
//

#include "table.h"

void AutoLineElement::onInputChar(EventContext &context, int ch) {
    if (ch == VK_RETURN) {
        auto line = context.getLineViewer();
        if (line.content() == _GT("if")) {
            context.replace(new SingleBlockElement());
            context.outer->relayout();
            context.reflow();
            context.redraw();
            auto newLine = context.getLineViewer();
            newLine.append(_GT(" ()"));
            EventContext &&ctx = context.enter();
            context.pos().setIndex(-2);
            ctx.push(CommandType::AddChar, CommandData(2, ' '));
            ctx.push(CommandType::AddChar, CommandData(3, '('));
            ctx.push(CommandType::AddChar, CommandData(4, ')'));
            ctx.focus();
            return;
        }
        if (line.content() == _GT("switch")) {
            context.replace(new SwitchElement());
            context.outer->relayout();
            context.reflow();
            context.redraw();
            auto newLine = context.getLineViewer();
            newLine.append(_GT(" ()"));
            EventContext &&ctx = context.enter().enter();
            context.pos().setIndex(-2);
            ctx.push(CommandType::AddChar, CommandData(6, ' '));
            ctx.push(CommandType::AddChar, CommandData(7, '('));
            ctx.push(CommandType::AddChar, CommandData(8, ')'));
            ctx.focus();
            return;
        }
    }
    LineElement::onInputChar(context, ch);
}
