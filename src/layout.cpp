//
// Created by Alex on 2019/6/27.
//

#include "layout.h"
#include "document.h"
void LayoutManager::reflow(EventContext context) {
    // 先把同级别的元素都安排一下
    int lineMaxHeight = 0, blockMaxWidth = 0;
    Offset offset = context.current()->getLogicOffset();
    int value;
    while (context.has()) {
        Element *current = context.current();
        current->onEnterReflow(context, offset);
        current->setLogicOffset(offset);
        switch (current->getDisplay()) {
            case Display::None:
                break;
            case Display::Inline:
                blockMaxWidth = 0;
                offset.x += current->getWidth(context);
                value = current->getHeight(context);
                if (value > lineMaxHeight) {
                    lineMaxHeight = value;
                }
                break;
            case Display::Line:
            case Display::Block:
                lineMaxHeight = 0;
                value = current->getWidth(context);
                if (value > blockMaxWidth) {
                    blockMaxWidth = value;
                }
                offset.x = 0;
                offset.y += current->getHeight(context);
                break;
        }
        current->onLeaveReflow(context);
        context.next();
    }
    // 再把父级别的元素都安排一下
    if (context.outer != nullptr) {
        if (lineMaxHeight) {
            context.outer->current()->setLogicWidth(offset.x);
            context.outer->current()->setLogicHeight(lineMaxHeight);
        }
        if (blockMaxWidth) {
            context.outer->current()->setLogicWidth(blockMaxWidth);
            context.outer->current()->setLogicHeight(offset.y);
        }
        reflow(*context.outer);
    } else {
        m_width = blockMaxWidth;
        m_height = offset.y;
    }

}

void LayoutManager::reflowAll(Document *doc) {
    EventContext context = EventContextBuilder::build(doc);
    context.init(doc, 0);
    reflowEnter(context);
}

void LayoutManager::reflowEnter(EventContext context) {
    int lineMaxHeight = 0, blockMaxWidth = 0, value = 0;
    Offset offset = context.current()->getLogicOffset();
    while (context.has()) {
        Element *current = context.current();
        current->onEnterReflow(context, offset);
        current->setLogicOffset(offset);
        switch (current->getDisplay()) {
            case Display::None:
                break;
            case Display::Inline:
                blockMaxWidth = 0;
                if (current->hasChild()) {
                    EventContext ctx = context.enter();
                    reflowEnter(ctx);
                    // inline里面不计行
                    //ctx.leave();
                }
                offset.x += current->getWidth(context);
                value = current->getHeight(context);
                if (value > lineMaxHeight) {
                    lineMaxHeight = value;
                }
                break;
            case Display::Line:
            case Display::Block:
                lineMaxHeight = 0;
                if (current->hasChild()) {
                    EventContext ctx = context.enter();
                    reflowEnter(ctx);
                    ctx.leave();
                }
                value = current->getWidth(context);
                if (value > blockMaxWidth) {
                    blockMaxWidth = value;
                }
                offset.x = 0;
                offset.y += current->getHeight(context);
                break;
        }
        current->onLeaveReflow(context);
        context.next();
    }
    if (context.outer != nullptr) {
        if (lineMaxHeight) {
            context.outer->current()->setLogicWidth(offset.x);
            context.outer->current()->setLogicHeight(lineMaxHeight);
        }
        if (blockMaxWidth) {
            context.outer->current()->setLogicWidth(blockMaxWidth);
            context.outer->current()->setLogicHeight(offset.y);
        }
    } else {
        m_width = blockMaxWidth;
        m_height = offset.y;
    }
}
