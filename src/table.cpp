//
// Created by Alex on 2019/8/4.
//

#include "table.h"

void LineElement::onRightButtonUp(EventContext &context, int x, int y) {
    context.getCaretManager()->focus(nullptr);
    context.replace(new SingleBlockElement())->free();
    context.outer->relayout();
    context.reflow();
    context.redraw();
    context.pos().setIndex(-1);
    context.enter().focus();
}

void LineElement::onInputChar(EventContext &context, int ch) {
    auto* caret = context.getCaretManager();
    TextCaretService service(Offset(4, 6), &context);
    auto line = context.getLineViewer();
    switch (ch) {
        case VK_BACK:
            if (service.index() > 0) {
                line.remove(service.index() - 1);
                service.moveLeft();
            } else {
                EventContext prev = context.nearby(-1);
                if (prev.tag().contain(_GT("Line"))) {
                    auto prevLine = prev.getLineViewer();
                    context.pos().setIndex(prevLine.length());
                    prevLine.append(line.c_str());
                    context.deleteLine();
                    erase(context);
                    prev.reflow();
                    prev.redraw();
                    prev.focus();
                    return;
                } else {
                    if (context.outer && context.outer->tag().contain(_GT("CodeBlock"))) {
                        if (context.outer->outer && context.outer->outer->tag().contain(_GT("Switch"))) {
                            EventContext *switchBlock = context.outer->outer;
                            //switchBlock->replace();
                        } else {
                            if (m_next == nullptr && m_prev == nullptr && context.getLineViewer().empty()) {
                                context.outer->replace(new SyntaxLineElement());
                                context.outer->outer->relayout();
                                context.outer->redraw();
                                context.outer->focus();
                                return;
                            }
                        }
                    }
                    context.pos().setIndex(-1);
                    caret->findPrev(TAG_FOCUS);
                }
                return;
            }
            break;
        case VK_RETURN: {
            if (line.content() == _GT("if")) {
                context.replace(new SingleBlockElement())->free();
                context.outer->relayout();
                context.reflow();
                context.redraw();
                auto newLine = context.getLineViewer();
                newLine.append(_GT("if ()"));
                context.pos().setIndex(-2);
                context.enter().focus();
                return;
            }
            insert(context);
            int idx = service.index();
            auto next = context.insertLine(1);
            next.append(line.c_str() + idx, line.length() - idx);
            line.remove(idx, line.length() - idx);
            context.reflow();
            context.redraw();
            context.pos().setIndex(0);
            caret->data().setIndex(0);
            caret->next();
            return;
        }
        default:
            line.insert(service.index(), ch);
            context.push(CommandType::Add, CommandData(service.index(), ch));
            service.moveRight();
            break;
    }
    service.commit();
    context.redraw();
}
