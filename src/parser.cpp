//
// Created by Alex on 2020/2/26.
//

#include "parser.h"
#include "event.h"
#include "table.h"
void SimpleParser::parse(EventContext *context, GParser::istream &in) {
    GString line;
    std::getline(in, line);
    context->getLineViewer().insert(context->pos().getIndex(), line.c_str());
    context->pos().setIndex(context->pos().getIndex() + (signed) line.length());
    EventContext current = *context;
    while (std::getline(in, line)) {
        current.insert(new AutoLineElement());
        current.next();
        int length = line.length();
        current.getLineViewer().append(line.c_str(), length);
        current.pos().setIndex(length);
    }
    context->reflow();
    current.focus(true);

}
