//
// Created by Administrator on 2019/6/27.
//

#include "layout.h"
#include "document.h"
void LayoutManager::reflow(Root *sender) {
    NOT_REACHED();
}

void LayoutManager::reflow(RelativeElement *sender) {
    // 先把同级别的元素都安排一下
    Offset offset;
    RelativeElement *element = sender;
    while (element->m_next != nullptr) {
        element->setLogicOffset(offset);
        switch (element->getDisplay()) {
            case Display::None:
                break;
            case Display::Inline:
                offset.x += element->getWidth();
                break;
            case Display::Block:
                offset.x = 0;
                offset.y += element->getHeight();
                break;
        }
    }
    // 再把父亲安排一下
    sender->m_parent->reflow(document->getContext());
}

void LayoutManager::reflow(Document *sender) {
    // 重排Document... 好像没啥要干的啊
}

void LayoutManager::reflow(Element *sender) {

}
