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
            if (context.isHead() && index == 0) {
                if (context.outer && context.outer->tag().contain(TAG("CodeBlock"))) {
                    EventContext ctx;
                    if (context.outer->outer && context.outer->outer->tag().contain(TAG("Switch"))) {
                        ctx = context.outer->outer->nearby(-1);
                    } else {
                        ctx = context.outer->nearby(-1);
                    }
                    if (ctx.getLineViewer().empty()) {
                        EventContext prev = ctx.nearby(-1);
                        ctx.remove();
                        prev.update();
                        if (EventContext *inner = prev.nearby(1).findInnerFirst(TAG_FOCUS)) {
                            inner->focus(false, true, &context);
                        }
                        return;
                    }
                }
            }
            if (context.isTail() && index == 0) {
                if (context.nearby(-1).tag().contain(TAG("Condition"))) {
                    caret->data().setIndex(-1);
                    caret->findPrev(TAG_FOCUS);
                    return;
                }
            }
            if (line.getSpaceCount() == index) {
                auto *prev = context.findPrev(TAG_FOCUS);
                if (!prev) {
                    return;
                }
                if (prev->tag().contain(TAG_LINE)) {
                    context.getDocContext()->pushStart();
                    line.remove(0, index);
                    int prevLength = prev->getLineViewer().length();
                    context.combineLine(-1);
                    erase(context);
                    prev->reflow();
                    prev->redraw();
                    prev->pos().setIndex(prevLength);
                    prev->focus(false);
                    context.getDocContext()->pushEnd();
                } else {
                    prev->focus(false);
                }
                return;
            }
        }
        if (ch == VK_RETURN) {
            if (line.flags() & LineFlagExpand) {
                context.pos().setIndex(0);
                caret->findNext(TAG_FOCUS);
                return;
            }
            if (context.isHead() && index == 0) {
                if (context.outer && context.outer->tag().contain(TAG("CodeBlock"))) {
                    EventContext ctx;
                    if (context.outer->outer && context.outer->outer->tag().contain(TAG("Switch"))) {
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
                    if (EventContext *inner = ctx.nearby(2).findInnerFirst(TAG_FOCUS)) {
                        inner->focus(false, true);
                    }
                    return;
                }
            }
            if (context.isTail() && index == line.length()) {
                if (context.outer && context.outer->tag().contain(TAG("Loop"))) {
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
                if (EventContext *inner = context.findInnerFirst(TAG_FOCUS)) {
                    auto newLine = inner->getLineViewer();
                    newLine.append(_GT(" ()"));
                    inner->pos().setIndex(-2);
                    inner->focus(false);
                    return;
                }
            }
            if (!line.empty()) {
                context.getDocContext()->pushStart();
                int indent = line.getSpaceCount();
                if (line.back() == _GT('{') && index > indent) {
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
        }
        if (ch == VK_TAB) {
            static const GChar *jumps = _GT("'\",()<>;{}:");
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
                //caret->data().setIndex(caret->data())
                context.pos().setIndex(context.pos().getIndex() + 1);
                context.focus(false);
                return;
            }
        }
    }
    LineElement::onInputChar(context, state, ch);
    if (auto *mgr = context.document()->getDocumentManager()->LSPManager()) {
        mgr->onTrigger(context, ch);
    }
    if (EventContext *current = caret->getEventContext()) { // (<current>) 跳过右括号
        line = current->getLineViewer();
        int index = current->pos().getIndex();
        if (auto *left = gstrchr(lchar, ch)) {
            size_t char_idx = left - lchar;
            line.insert(index, rchar[char_idx]);
            current->focus(false);
        }
        if (auto *res = ASTElement::parseLines(context)) {
            res->dump(0);
        }

    }

}

void AutoLineElement::onMouseHover(EventContext &context, int x, int y) {
    if (auto *mgr = context.document()->getDocumentManager()->LSPManager()) {
        int index = TextCaretService::GetIndex(context, padding(), x, y);
        mgr->onHover({context.line(), index});
    }
}

void AutoLineElement::onMouseLeave(EventContext &context, int x, int y) {
    printf("leave\n");
}

void TextElement::onRedraw(EventContext &context) {
    auto canvas = context.getCanvas();
    auto state = context.outer->getSelectionState();
    auto *table = context.getOuter(2)->cast<TableElement>();
    int row = context.outer->index, col = context.index;

    GStyle paint = context.getStyle(StyleTableBackground);
    paint.setColor(table->getBackgroundColor(row, col));
    canvas.drawRect(canvas.bound(0.5, 0.5), paint);

    if (context.isSelectedSelf()) {
        Offset start = context.relOffset(context.getDocContext()->getSelectStart());
        Offset end = context.relOffset(context.getDocContext()->getSelectEnd());
        DrawUtil::DrawSlection(context, start, end);
    } else {
        if (state == SelectionSelf && context.outer->selectedCount() > 1) {
            canvas.drawRect(canvas.bound(0.5, 0.5), StyleTableSelected);
        }
        if (state != SelectionNone && state != SelectionSelf) {
            canvas.drawRect(canvas.bound(0.5, 0.5), StyleTableSelected);
        }
    }
    canvas.drawRect(canvas.bound(0.5, 0.5), StyleTableBorder);
    if (m_radio) {
        if (m_data.empty()) {
            return;
        }
        canvas.translate((float) context.width() / 2, context.height() - 5);
        canvas.rotate(40);
        GStyle paint;
        paint.setColor(table->getForegroundColor(row, col));
        paint.setWidth(3);
        paint.setAntiAlias(true);
        paint.setStrokeCap(GStyle::CapRound);
        paint.setBlur(SK_ColorLTGRAY, 3.5f, 0, 0);
        canvas.drawLine(0, 0, -8, 0, paint);
        canvas.rotate(90);
        canvas.drawLine(0, 0, -13, 0, paint);
        return;
    }

    GStyle style = context.getStyle(StyleTableFont);
    style.setColor(table->getForegroundColor(row, col));
    canvas.translate(location());
    canvas.drawText(m_data.c_str(), m_data.size() * sizeof(GChar), 0, 0, style);

}
