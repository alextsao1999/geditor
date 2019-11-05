//
// Created by Alex on 2019/6/27.
//

#include "layout.h"
#include "document.h"
#include "table.h"
void LayoutManager::reflow(EventContext context, bool init) {
    LayoutContext layoutContext;
    // 先把同级别的元素都安排一下
    if (!context.has()) {
        return;
    }
    Offset offset = context.current()->getLogicOffset();
    while (context.has()) {
        Element *current = context.current();
        current->onEnterReflow(context, offset);
        Offset self = offset;
        Display display = current->getDisplay();
        m_layouts[display](this, display, context, layoutContext, self, offset, init);
        current->setLogicOffset(self);
        current->onLeaveReflow(context);
        context.next();
    }
    if (context.outer) {
        if (layoutContext.lineMaxHeight) {
            context.outer->setLogicWidth(offset.x);
            context.outer->setLogicHeight(layoutContext.lineMaxHeight);
        }
        if (layoutContext.blockMaxWidth) {
            context.outer->setLogicWidth(layoutContext.blockMaxWidth);
            context.outer->setLogicHeight(offset.y);
        }
        if (!init) {
            // 再把父级别的元素都安排一下
            reflow(*context.outer, false);
        }
    } else {
        m_width = layoutContext.blockMaxWidth;
        m_height = offset.y;
    }

}

void LayoutManager::reflowAll(Document *doc) {
    EventContext context = EventContextBuilder::build(doc);
    context.init(doc);
    context.reflow(true);
}
Layout(LayoutDisplayNone) {}
Layout(LayoutDisplayInline) {
    layoutContext.blockMaxWidth = 0;
    next.x += context.width();
    int value = context.height();
    if (value > layoutContext.lineMaxHeight) {
        layoutContext.lineMaxHeight = value;
    }
}
Layout(LayoutDisplayBlock) {
    if (init) {
        sender->reflow(context.enter(), init);
    }
    layoutContext.lineMaxHeight = 0;
    int value = context.width();
    if (value > layoutContext.blockMaxWidth) {
        layoutContext.blockMaxWidth = value;
    }
    next.x = 0;
    next.y += context.height();
}
Layout(LayoutDisplayLine) {
    if (init) {
        sender->reflow(context.enter(), init);
    }
    CallDisplayFunc(LayoutDisplayBlock);
}
Layout(LayoutDisplayTable) {
    if (init) {
        sender->reflow(context.enter(), true);
        Buffer<int> maxWidthBuffer;
        for_context(row, context) {
            for_context(col, row) {
                int width = col.logicWidth();
                if (width > maxWidthBuffer[col.index]) {
                    maxWidthBuffer[col.index] = width;
                }
            }
        }
        for (auto iter = maxWidthBuffer.iter(); iter.has(); iter.next()) {
            for_context(table_row, context) {
                table_row.enter(iter.index()).setLogicWidth(iter.current());
            }
        }
        maxWidthBuffer.clear();
        sender->reflow(context.enter(), true);
    }
    if (context.outer && context.outer->current()->getDisplay() == DisplayRow) {
        CallDisplayFunc(LayoutDisplayInline);
    } else {
        CallDisplayFunc(LayoutDisplayBlock);
    }
}
Layout(LayoutDisplayRow) {
    if (init) {
        sender->reflow(context.enter(), true);
    }
    CallDisplayFunc(LayoutDisplayBlock);
}
