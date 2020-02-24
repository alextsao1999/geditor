//
// Created by Alex on 2019/12/4.
//

#include "line_index.h"
#include "document.h"
void LineCounter::increase(EventContext *context, int num) {
    line += num;
    //doc->m_context.m_textBuffer.getLine();
}

void LineCounter::decrease(EventContext *context, int num) {
    line -= num;
}
void LineCounter::increase(EventContext *context) {
    line += context->element->getLineNumber();
}

void LineCounter::decrease(EventContext *context) {
    line -= context->element->getLineNumber();
}
