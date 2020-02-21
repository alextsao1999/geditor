//
// Created by Alex on 2019/8/4.
//

#include "table.h"
#include "doc_manager.h"
void AutoLineElement::onInputChar(EventContext &context, SelectionState state, int ch) {
    static const GChar *lchar = _GT("([{\"'");
    static const GChar *rchar = _GT(")]}\"'");
    auto line = context.getLineViewer();
    auto caret = context.getCaretManager();
    if (state == SelectionNone) {
        int index = context.pos().getIndex();
        if (ch == VK_BACK) {
            if (line.getSpaceCount() == index) {
                EventContext prev = context.nearby(-1);
                if (prev.tag().contain(_GT("Line"))) {
                    context.getDocContext()->pushStart();
                    line.remove(0, index);
                    int prevLength = prev.getLineViewer().length();
                    context.combineLine(-1);
                    erase(context);
                    prev.reflow();
                    prev.redraw();
                    caret->data().setIndex(prevLength);
                    prev.focus(); // 会释放context
                    context.getDocContext()->pushEnd();
                    return;
                }
            }
            if (context.isHead() && index == 0) {
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
            if (context.isTail() && index == 0) {
                if (context.nearby(-1).tag().contain(_GT("Condition"))) {
                    caret->data().setIndex(-1);
                    caret->findPrev(TAG_FOCUS);
                    return;
                }
            }
        }
        if (ch == VK_RETURN) {
            if (context.isHead() && index == 0) {
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
            if (context.isTail() && index == line.length()) {
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
                EventContext *inner = context.findInnerFirst(TAG_FOCUS);
                auto newLine = inner->getLineViewer();
                newLine.append(_GT(" ()"));
                inner->pos().setIndex(-2);
                inner->focus(false);
                return;
            }
            context.getDocContext()->pushStart();
            int indent = line.getSpaceCount();
            if (line.back() == _GT('{')) {
                indent += 4;
            }
            bool newLine = false;
            if (line.charAt(index) == _GT('}')) {
                newLine = true;
            }
            GChar str[255] = {_GT('\0')};
            for (int i = 0; i < indent; ++i) {
                gstrcat(str, _GT(" "));
            }
            insert(context);
            context.breakLine(0, context.pos().getIndex());
            context.nearby(1).getLineViewer().insert(0, str);
            context.pos().setIndex(indent);
            if (newLine) {
                insert(context);
                context.pos().setIndex(indent + 4);
                gstrcat(str, _GT("    "));
                context.nearby(1).getLineViewer().insert(0, str);
            }
            context.reflow();
            context.redraw();
            context.next();
            context.focus(false);
            context.getDocContext()->pushEnd();
            return;
        }
        if (ch == VK_TAB) {
            static const GChar *jumps = _GT("'\",()<>;{}");
            while (index < line.length()) {
                if (gstrchr(jumps, line.charAt(index))) {
                    context.pos().setIndex(index + 1);
                    context.focus(false);
                    return;
                }
                index++;
            }
            context.getDocContext()->pushStart();
            for (int i = 0; i < 4; ++i) {
                LineElement::onInputChar(context, state, VK_SPACE);
            }
            context.getDocContext()->pushEnd();
            return;
        }
        if (auto *right = gstrchr(rchar, ch)) {
            if (line.charAt(context.pos().getIndex()) == ch) {
                context.pos().setIndex(context.pos().getIndex() + 1);
                context.focus(false);
                return;
            }
        }
    }
    LineElement::onInputChar(context, state, ch);
    if (state != SelectionNone) {
        return;
    }
    EventContext *current = caret->getEventContext();
    if (auto *mgr = current->document()->getDocumentManager()) {
        mgr->onTrigger(context, ch);
    }
    if (current) {
        line = current->getLineViewer();
        int index = context.pos().getIndex();
        if (auto *left = gstrchr(lchar, ch)) {
            size_t char_idx = left - lchar;
            line.insert(index, rchar[char_idx]);
            current->focus(false);
        }
        if (current->isHead() && current->outer && current->outer->tag().contain(_GT("Single"))) {
            if (line.content().substr(0, 2) != _GT("if")) {
                EventContext single = *current->outer;
                single.seperate();
                single.outer->update();
                single.focus(true, true);
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
