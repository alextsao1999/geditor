//
// Created by Alex on 2019/6/27.
//

#include "layout.h"
#include "document.h"
#include "table.h"
void LayoutManager::ReflowAll(Document *doc) {
    EventContext context = EventContextBuilder::build(doc);
    context.init(doc);
    context.reflow(true);
}

void LayoutManager::reflow(EventContext context, bool relayout) {
    if (!context.has()) {
        return;
    }
    // 把自身安排一下
    LayoutContext layoutContext;
    Element *current = context.current();
    Offset offset = current->getLogicOffset();
    Offset self = offset;
    Display display = current->getDisplay();
    m_layouts[display](context, this, display, layoutContext, self, offset, relayout);
    context.next();
    // 再把同级别的元素都安排一下
    while (context.has()) {
        current = context.current();
        current->onEnterReflow(context, offset);
        self = offset;
        display = current->getDisplay();
        m_layouts[display](context, this, display, layoutContext, self, offset, relayout);
        current->setLogicOffset(self);
        current->onLeaveReflow(context);
        context.next();
    }
    if (context.outer) {
        if (layoutContext.lineMaxHeight) {
            context_on(*context.outer, FinishReflow, offset.x, layoutContext.lineMaxHeight);
        }
        if (layoutContext.blockMaxWidth) {
            context_on(*context.outer, FinishReflow, layoutContext.blockMaxWidth, offset.y);
        }
        if (!relayout) { // 再把父级别的元素都安排一下
            reflow(*context.outer, false);
        }
    } else {
        //m_width = layoutContext.blockMaxWidth;
        //m_height = offset.y;
    }
}

void LayoutManager::relayout(EventContext context) {
    if (!context.has()) {
        return;
    }
    LayoutContext layoutContext;
    Element *current = context.current();
    Offset offset = current->getLogicOffset();
    Display display = context.display();
    Offset self = offset;
    current->onEnterReflow(context, offset);
    m_layouts[display](context, this, display, layoutContext, self, offset, true);
    current->setLogicOffset(self);
    current->onLeaveReflow(context);
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
    if (relayout) {
        sender->reflow(context.enter(), true);
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
    if (relayout) {
        sender->reflow(context.enter(), relayout);
    }
    UseDisplayFunc(LayoutDisplayBlock);
}

Layout(LayoutDisplayTable) {
    if (relayout) {
        Buffer<int> MaxWidthBuffer;
        Buffer<int> MaxHeightBuffer;
        for_context(row, context) {
            for_context(col, row) {
                col.relayout();
                int width = col.minWidth();
                int height = col.minHeight();
                if (width > MaxWidthBuffer[col.index]) MaxWidthBuffer[col.index] = width;
                if (height > MaxHeightBuffer[row.index]) MaxHeightBuffer[row.index] = height;
            }
        }
        Offset pos_row(0, 0);
        for_context(row, context) {
            row.current()->setLogicOffset(pos_row);
            Offset pos_col(0, 0);
            for_context(col, row) {
                col.current()->setLogicOffset(pos_col);
                col.setLogicWidth(MaxWidthBuffer[col.index]);
                col.setLogicHeight(MaxHeightBuffer[row.index]);
                //context_on(col, FinishReflow, MaxWidthBuffer[col.index], MaxHeightBuffer[row.index]);
                pos_col.x += MaxWidthBuffer[col.index];
            }
            context_on(row, FinishReflow, pos_col.x, MaxHeightBuffer[row.index]);
            pos_row.y += MaxHeightBuffer[row.index];
        }
        int table_width = 0;
        for (auto iter = MaxWidthBuffer.iter(); iter.has(); iter.next()) {
            table_width += iter.current();
        }
        context_on(context, FinishReflow, table_width, pos_row.y);
    }
    if (context.outer && context.outer->display() == DisplayRow) {
        UseDisplayFunc(LayoutDisplayInline);
    } else {
        UseDisplayFunc(LayoutDisplayBlock);
    }
}

Layout(LayoutDisplayRow) {
    if (relayout) {

    }
    UseDisplayFunc(LayoutDisplayBlock);
}

Layout(LayoutDisplayCustom) {
    CallDisplayFunc(context.current()->onReflow);
}
