//
// Created by Alex on 2019/12/4.
//

#ifndef GEDITOR_LINE_INDEX_H
#define GEDITOR_LINE_INDEX_H

class EventContext;
class LineCounter {
public:
    int line = 0;
    int offset = 0;
    LineCounter() = default;
    LineCounter(int line, int offset) : line(line), offset(offset) {}
    inline LineCounter operator+(const LineCounter &rv) { return {line + rv.line, offset + rv.offset}; }
    inline LineCounter operator-(const LineCounter &rv) { return {line - rv.line, offset - rv.offset}; }
    void increase(EventContext *context, int num);
    void decrease(EventContext *context, int num);
};


#endif //GEDITOR_LINE_INDEX_H
