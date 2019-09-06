//
// Created by Administrator on 2019/6/27.
//

#include "layout.h"
#include "document.h"
#define checkWidth(width)
void LayoutManager::reflow(EventContext context) {
    // 先把同级别的元素都安排一下
    Offset offset = context.current()->getLogicOffset();
    int value;
    while (context.has()) {
        Element *current = context.current();
        current->setLogicOffset(offset);
        switch (current->getDisplay()) {
            case Display::None:
                break;
            case Display::Inline:
                value = current->getWidth(context);
                offset.x += value;
                if (offset.x > m_width) {
                    m_width = offset.x;
                }
                if (m_minWidth > value || !m_minWidth)
                    m_minWidth = value;
                value = current->getHeight(context);
                if (m_minHeight > value  || !m_minHeight)
                    m_minHeight = value;
                break;
            case Display::Line:
            case Display::Block:
                value = current->getWidth(context);
                if (offset.x +  value> m_width)
                    m_width = offset.x + value;
                if (m_minWidth > value  || !m_minWidth)
                    m_minWidth = value;
                value = current->getHeight(context);
                if (m_minHeight > value  || !m_minHeight)
                    m_minHeight = value;
                offset.x = 0;
                offset.y += value;
                break;
        }
        context.next();
    }
    if (offset.y > m_height) {
        m_height = offset.y;
    }
    // 再把父级别的元素都安排一下
    if (context.outer != nullptr && context.outer->has()) {
        reflow(*context.outer);
    }

}

void LayoutManager::reflowAll(Document *doc) {
    EventContext context = EventContextBuilder::build(doc);
    context.set(doc, 0);
    reflowEnter(context);
}

void LayoutManager::reflowEnter(EventContext context) {
    Offset offset = context.current()->getLogicOffset();
    while (context.has()) {
        Element *current = context.current();
        current->setLogicOffset(offset);
        switch (current->getDisplay()) {
            case Display::None:
                break;
            case Display::Inline:
                offset.x += current->getWidth(context);
                break;
            case Display::Line:
            case Display::Block:
                if (current->hasChild()) {
                    EventContext ctx = context.enter();
                    reflowEnter(ctx);
                }
                offset.x = 0;
                offset.y += current->getHeight(context);
                break;
        }
        context.next();
    }

}
