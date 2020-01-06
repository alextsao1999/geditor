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
            newLine.append(_GT("if ()"));
            context.pos().setIndex(-2);
            context.enter().focus();
            return;
        }
        if (line.content() == _GT("switch")) {
            context.replace(new SwitchElement());
            context.outer->relayout();
            context.reflow();
            context.redraw();
            auto newLine = context.getLineViewer();
            newLine.append(_GT("switch ()"));
            context.pos().setIndex(-2);
            context.enter().enter().focus();
            return;
        }
    }
    LineElement::onInputChar(context, ch);
}
