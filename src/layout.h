//
// Created by Alex on 2019/6/27.
//

#ifndef GEDITOR_LAYOUT_H
#define GEDITOR_LAYOUT_H

#include "common.h"

struct Root;
struct RelativeElement;
struct Document;
struct Element;
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
/*
    int m_minWidth{0};
    int m_minHeight{0};
    int m_maxHeight{0};
*/
public:
    void reflow(EventContext context);
    void reflowAll(Document *doc);
    void reflowEnter(EventContext context);
    int getHeight() {
        return m_height;
    }
    int getWidth() {
        return m_width;
    }
};


#endif //GEDITOR_LAYOUT_H
