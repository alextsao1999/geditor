//
// Created by Administrator on 2019/6/27.
//

#ifndef GEDITOR_LAYOUT_H
#define GEDITOR_LAYOUT_H

struct Element;
struct RelativeElement;
struct Document;
enum class SelectionState {
    SelectionNone,
    SelectionStart,
    SelectionInside,
    SelectionEnd,
    SelectionBoth
};

struct Position {

};

class LayoutManager {
public:
    virtual void reflow() {}
    virtual void reflow(RelativeElement *element) {}
    virtual void redraw(Element *element) {}

private:
    Document *m_document = nullptr;


};


#endif //GEDITOR_LAYOUT_H
