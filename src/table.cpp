//
// Created by Alex on 2019/8/4.
//

#include "table.h"

void AutoLineElement::onInputChar(EventContext &context, SelectionState state, int ch) {
    if (context.doc->m_onInputChar) {
        ch = context.doc->m_onInputChar(&context, state, ch);
        if (ch == 0) {
            return;
        }
    }
    auto line = context.getLineViewer();
    if (ch == VK_RETURN) {
        Element *replace = nullptr;
        if (line.content() == _GT("if")) {
            replace = new SingleBlockElement(2);
        }
        if (line.content() == _GT("loop")) {
            replace = new LoopBlockElement(2);
        }
        if (line.content() == _GT("switch")) {
            replace = new SwitchElement(3);
        }
        if (replace) {
            if (context.isTail()) {
                insert(context);
            }
            context.replace(replace);
            context.outer->update();
            auto newLine = context.getLineViewer();
            newLine.append(_GT(" ()"));
            EventContext *inner = context.findInnerFirst(TAG_FOCUS);
            inner->pos().setIndex(-2);
            inner->push(CommandType::AddChar, CommandData(2, ' '));
            inner->push(CommandType::AddChar, CommandData(3, '('));
            inner->push(CommandType::AddChar, CommandData(4, ')'));
            inner->focus(false);
            return;
        }
    }
    LineElement::onInputChar(context, state, ch);
}
