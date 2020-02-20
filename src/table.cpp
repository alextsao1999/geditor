//
// Created by Alex on 2019/8/4.
//

#include "table.h"
#include "doc_manager.h"
void AutoLineElement::onInputChar(EventContext &context, SelectionState state, int ch) {
    auto line = context.getLineViewer();
    auto caret = context.getCaretManager();
    if (state == SelectionNone) {
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
            if (context.isTail() && context.pos().index == 0) {
                if (context.nearby(-1).tag().contain(_GT("Condition"))) {
                    caret->data().setIndex(-1);
                    caret->findPrev(TAG_FOCUS);
                    return;
                }
            }
        }
        if (ch == VK_RETURN) {
            if (context.isHead() && context.pos().index == 0) {
                if (context.outer && context.outer->tag().contain(_GT("CodeBlock"))) {
                    EventContext ctx;
                    if (context.outer->outer && context.outer->outer->tag().contain(_GT("Switch"))) {
                        if (context.outer->isHead()) {
                            ctx = context.outer->outer->nearby(-1);
                        } else {
                            LineElement::onInputChar(context, state, ch);
                            return;
                        }
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
            if (context.isTail() && context.pos().index == line.length()) {
                if (context.outer && context.outer->tag().contain(_GT("Loop"))) {
                    context.outer->insert(copy());
                    context.outer->update();
                    context.outer->nearby(1).focus();
                    return;
                }
            }
            Element *replace = nullptr;
            if (line.content() == _GT("if")) {
                replace = new SingleBlockElement(2);
            }
            if (line.content() == _GT("loop")) {
                replace = new LoopBlockElement(3);
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
                inner->focus(false);
                return;
            }
        }
        if (ch == VK_TAB) {
            auto index = context.pos().getIndex();
            static std::set<GChar> sets = {_GT('"'), _GT(','), _GT(')')};
            while (index < line.length()) {
                if (sets.count(line.charAt(index))) {
                    context.pos().setIndex(index + 1);
                    context.focus(false);
                    return;
                }
                index++;
            }
            for (int i = 0; i < 4; ++i) {
                LineElement::onInputChar(context, state, VK_SPACE);
            }
            return;
        }
    }
    LineElement::onInputChar(context, state, ch);
    if (ch == '(') {
        line.insert(context.pos().getIndex(), ')');
    }
    if (auto *mgr = context.document()->getDocumentManager()) {
        mgr->onTrigger(context, ch);
    }
    if (state == SelectionNone) {
        EventContext *current = caret->getEventContext();
        if (current) {
            line = current->getLineViewer();
            if (current->isHead() && current->outer && current->outer->tag().contain(_GT("Single"))) {
                if (line.content().substr(0, 2) != _GT("if")) {
                    EventContext single = *current->outer;
                    single.seperate();
                    single.outer->update();
                    single.focus(true, true);
                    return;
                }
            }
        }
    }
}

void AutoLineElement::onLeftButtonDown(EventContext &context, int x, int y) {
    LineElement::onLeftButtonDown(context, x, y);
    if (auto *mgr = context.document()->getDocumentManager()) {
        mgr->onSignatureHelp(context.position());
    }

}

void AutoLineElement::onRightButtonDown(EventContext &context, int x, int y) {
    if (auto *mgr = context.document()->getDocumentManager()) {
        int index = TextCaretService::GetIndex(context, {4, 6}, x, y);
        mgr->onGoToDeclaration({context.getCounter().line, index});
    }

}

void AutoLineElement::onRightDoubleClick(EventContext &context, int x, int y) {
    if (auto *mgr = context.document()->getDocumentManager()) {
        CompletionContext ctx;
        ctx.triggerKind = CompletionTriggerKind::Invoked;
        mgr->onComplete(context.position(), ctx);
    }

}

void AutoLineElement::onMouseHover(EventContext &context, int x, int y) {
    if (auto *mgr = context.document()->getDocumentManager()) {
        int index = TextCaretService::GetIndex(context, {4, 6}, x, y);
        mgr->onHover({context.getCounter().line, index});
    }
}
