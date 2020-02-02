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
    if (ch == VK_BACK) {
        if (context.isHead() && context.pos().index == 0) {
            if (context.outer && context.outer->tag().contain(_GT("CodeBlock"))) {
                EventContext ctx;
                if (context.outer->outer && context.outer->outer->tag().contain(_GT("Switch"))) {
                    ctx = context.outer->outer->nearby(-1);
                } else {
                    ctx = context.outer->nearby(-1);
                }
                if (ctx.getLineViewer().empty()) {
                    EventContext prev = ctx.nearby(-1);
                    ctx.remove();
                    prev.update();
                    EventContext *inner = prev.nearby(1).findInnerFirst(TAG_FOCUS);
                    inner->focus(false, true);
                    return;
                }
            }
        }

    }
    if (ch == VK_RETURN) {
        if (context.isHead() && context.pos().index == 0) {
            if (context.outer && context.outer->tag().contain(_GT("CodeBlock"))) {
                EventContext ctx;
                if (context.outer->outer && context.outer->outer->tag().contain(_GT("Switch"))) {
                    ctx = context.outer->outer->nearby(-1);
                } else {
                    ctx = context.outer->nearby(-1);
                }
                ctx.insert(copy());
                ctx.update();
                EventContext *inner = ctx.nearby(2).findInnerFirst(TAG_FOCUS);
                inner->focus(false, true);
                return;
            }
        }
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
