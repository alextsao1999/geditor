//
// Created by Alex on 2019/6/27.
//

#ifndef GEDITOR_LAYOUT_H
#define GEDITOR_LAYOUT_H

#include "common.h"

typedef int Display;
enum {
    DisplayNone,
    DisplayInline,
    DisplayBlock,
    DisplayLine,
    DisplayTable,
    DisplayRow,
};

class Root;
class RelativeElement;
class Document;
class Element;
struct EventContext;
enum class SelectionState {
    SelectionNone,
    SelectionStart,
    SelectionInside,
    SelectionEnd,
    SelectionBoth
};
class LayoutManager {
private:
    int m_width = 500;
    int m_height = 500;
public:
    void reflow(EventContext context);
    void reflowAll(Document *doc);
    void reflowEnter(EventContext context);
    int getHeight() { return m_height; }
    int getWidth() { return m_width; }
};


#endif //GEDITOR_LAYOUT_H
