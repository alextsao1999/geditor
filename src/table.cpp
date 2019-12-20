//
// Created by Alex on 2019/8/4.
//

#include "table.h"

void LineElement::onRightButtonUp(EventContext &context, int x, int y) {
    context.replace(new SingleBlockElement());
    //
    context.reflow();
    context.redraw();
}
