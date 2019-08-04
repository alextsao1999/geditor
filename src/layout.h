//
// Created by Administrator on 2019/6/27.
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
    Document *document;
public:
    explicit LayoutManager(Document *document) : document(document) {}
    void reflow(EventContext context);
};


#endif //GEDITOR_LAYOUT_H
