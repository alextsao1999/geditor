//
// Created by Alex on 2019/8/4.
//

#include "table.h"

void LineElement::onRightButtonUp(EventContext &context, int x, int y) {
    Element *ele = context.replace(nullptr);
    context.outer->relayout();
    context.redraw();
    context.remove(ele);
}
