//
// Created by Alex on 2019/6/27.
//

#include "layout.h"
#include "document.h"
#include "table.h"
LayoutManager::LayoutManager(RenderManager *renderManager) : m_renderManager(renderManager) {}

void LayoutManager::ReflowAll(Document *doc) {
    EventContext context = EventContextBuilder::build(doc);
    context.init(doc);
    doc->m_context.m_layoutManager.reflow(context, true, true);
    //context.reflow(true);
}

void LayoutManager::reflow(EventContext context, bool relayout, bool outset) {
    if (!context.has()) {
        return;
    }
    // 把自身安排一下
    LayoutContext layoutContext;
    Element *current = context.current();
    Offset offset = current->getLogicOffset();
    if (outset) {
        current->onEnterReflow(context, offset);
    }
    Offset self = offset;
    Display display = current->getDisplay();
    m_layouts[display](context, this, display, layoutContext, self, offset, relayout);
    if (outset) {
        current->setLogicOffset(self);
        current->onLeaveReflow(context);
    }
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
        m_height = offset.y;
        m_renderManager->setVertScroll(m_height);
    }
}

void LayoutManager::relayout(EventContext context) {
    if (!context.has()) {
        return;
    }
    LayoutContext layoutContext;
    Element *current = context.current();
    Offset offset = current->getLogicOffset();
    //current->onEnterReflow(context, offset);
    Display display = context.display();
    Offset self = offset;
    m_layouts[display](context, this, display, layoutContext, self, offset, true);
    current->setLogicOffset(self);
    //current->onLeaveReflow(context);
}

Layout(LayoutDisplayNone) {}
Layout(LayoutDisplayInline) {
    layoutContext.blockMaxWidth = 0;
    next.x += context.width();
    uint32_t value = context.height();
    if (value > layoutContext.lineMaxHeight) {
        layoutContext.lineMaxHeight = value;
    }
}
Layout(LayoutDisplayBlock) {
    if (relayout) {
        sender->reflow(context.enter(), true, true);
    }
    layoutContext.lineMaxHeight = 0;
    uint32_t value = context.width();
    if (value > layoutContext.blockMaxWidth) {
        layoutContext.blockMaxWidth = value;
    }
    next.x = 0;
    next.y += context.height();
}
Layout(LayoutDisplayLine) {
    if (relayout) {
        sender->reflow(context.enter(), true, true);
    }
    UseDisplayFunc(LayoutDisplayBlock);
}
Layout(LayoutDisplayTable) {
    if (relayout) {
        Buffer<uint32_t> MaxWidthBuffer;
        Buffer<uint32_t> MaxHeightBuffer;
        for_context(row, context) {
            for_context(col, row) {
                col.relayout();
                uint32_t width = col.minWidth();
                uint32_t height = col.minHeight();
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
            //row.setLogicWidth(pos_col.x);
            //row.setLogicHeight(MaxHeightBuffer[row.index]);
            context_on(row, FinishReflow, pos_col.x, MaxHeightBuffer[row.index]);
            pos_row.y += MaxHeightBuffer[row.index];
        }
        uint32_t table_width = 0;
        for (auto iter = MaxWidthBuffer.iter(); iter.has(); iter.next()) {
            table_width += iter.current();
        }
        context_on(context, FinishReflow, table_width, pos_row.y);
    }
    if (context.parent().display() == DisplayRow) {
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
