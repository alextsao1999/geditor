//
// Created by Alex on 2019/6/27.
//

#include "layout.h"
#include "document.h"
#include "table.h"
LayoutManager::LayoutManager(RenderManager *renderManager) : m_renderManager(renderManager) {}

void LayoutManager::ReflowAll(Document *doc) {
    EventContext context(doc);
    doc->m_context.m_layoutManager.reflow(context.enter(), true);
    //context.reflow(true);
}

void LayoutManager::reflow(EventContext context, bool relayout) {
    if (!context.has()) {
        return;
    }
    // 把自身安排一下
    LayoutContext layoutContext;
    Element *current = context.current();
    Offset offset;
    bool outset = context.isHead();
    if (outset) {
        // 从起始开始 offset 为 {0, 0}
        current->onEnterReflow(context, offset);
    } else {
        offset = current->getLogicOffset();
    }
    Offset self = offset;
    Display display = current->getDisplay();
    if (relayout) current->onRelayout(context, this);
    m_layouts[display](context, this, display, layoutContext, self, offset);
    if (outset) current->setLogicOffset(self);
    current->onLeaveReflow(context, offset);
    context.next();
    // 再把同级别的元素都安排一下
    while (context.has()) {
        current = context.current();
        current->onEnterReflow(context, offset);
        self = offset;
        display = current->getDisplay();
        if (relayout) current->onRelayout(context, this);
        m_layouts[display](context, this, display, layoutContext, self, offset);
        current->setLogicOffset(self);
        current->onLeaveReflow(context, offset);
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
    }
}

Layout(LayoutDisplayNone) {}
Layout(LayoutDisplayAbsolute) {}
Layout(LayoutDisplayInline) {
    layoutContext.blockMaxWidth = 0;
    int before = next.x;
    next.x += context.width();
    uint32_t value = context.height();
    if (value > layoutContext.lineMaxHeight) {
        layoutContext.lineMaxHeight = value;
    }
    next.x = before;
}
Layout(LayoutDisplayBlock) {
    layoutContext.lineMaxHeight = 0;
    uint32_t value = context.width();
    if (value > layoutContext.blockMaxWidth) {
        layoutContext.blockMaxWidth = value;
    }
    //next.x = 0;
    next.y += context.height();
}
Layout(LayoutDisplayLine) {
    UseDisplayFunc(LayoutDisplayBlock);
}
Layout(LayoutDisplayTable) {
    if (context.parent().display() == DisplayRow) {
        UseDisplayFunc(LayoutDisplayInline);
    } else {
        UseDisplayFunc(LayoutDisplayBlock);
    }
}
Layout(LayoutDisplayRow) {
    UseDisplayFunc(LayoutDisplayBlock);
}
Layout(LayoutDisplayCustom) {
    CallDisplayFunc(context.current()->onReflow);
}
