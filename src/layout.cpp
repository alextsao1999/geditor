//
// Created by Administrator on 2019/6/27.
//

#include "layout.h"
#include "document.h"

void LayoutManager::reflow(EventContext context) {
    // 先把同级别的元素都安排一下
    Offset offset = context.current()->getLogicOffset();
    while (context.has()) {
        Element *current = context.current();
        current->setLogicOffset(offset);
        switch (current->getDisplay()) {
            case Display::None:
                break;
            case Display::Inline:
                offset.x += current->getWidth(context);
                if (offset.x > m_width) {
                    m_width = offset.x;
                }
                break;
            case Display::Block:
                if (offset.x + current->getWidth(context) > m_width) {
                    m_width = offset.x + current->getWidth(context);
                }
                offset.x = 0;
                offset.y += current->getHeight(context);
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
