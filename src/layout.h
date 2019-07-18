//
// Created by Administrator on 2019/6/27.
//

#ifndef GEDITOR_LAYOUT_H
#define GEDITOR_LAYOUT_H

struct Root;
struct RelativeElement;
struct Document;
enum class SelectionState {
    SelectionNone,
    SelectionStart,
    SelectionInside,
    SelectionEnd,
    SelectionBoth
};

class LayoutManager {
public:
    virtual void reflow() {}
    virtual void reflow(RelativeElement *element) {}
    virtual void redraw(Root *element) {}

};


#endif //GEDITOR_LAYOUT_H
