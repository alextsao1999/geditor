//
// Created by Alex on 2019/8/4.
//

#include "table.h"

void AutoLineElement::onInputChar(EventContext &context, SelectionState state, int ch) {
    auto line = context.getLineViewer();
    if (ch == VK_RETURN) {
        if (line.content() == _GT("if")) {
            context.replace(new SingleBlockElement(2));
            context.outer->update();
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
        if (line.content() == _GT("loop")) {
            context.replace(new LoopBlockElement(2));
            context.outer->update();
            auto newLine = context.getLineViewer();
            newLine.append(_GT(" ()"));
            EventContext &&ctx = context.enter();
            context.pos().setIndex(-2);
            ctx.push(CommandType::AddChar, CommandData(4, ' '));
            ctx.push(CommandType::AddChar, CommandData(5, '('));
            ctx.push(CommandType::AddChar, CommandData(6, ')'));
            ctx.focus();
            return;
        }
        if (line.content() == _GT("switch")) {
            context.replace(new SwitchElement(3));
            context.outer->update();
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
    LineElement::onInputChar(context, state, ch);
}
