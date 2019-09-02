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
                break;
            case Display::Block:
                offset.x = 0;
                offset.y += current->getHeight(context);
                break;
        }
        context.next();
    }
    // 再把父级别的元素都安排一下
    if (context.outer != nullptr && context.outer->has()) {
        reflow(*context.outer);
    }

}
