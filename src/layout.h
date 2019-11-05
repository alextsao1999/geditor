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
class LayoutManager;
struct EventContext;
struct Offset;
struct LayoutContext {
    int lineMaxHeight = 0;
    int blockMaxWidth = 0;
};
#define LayoutArgs() (LayoutManager *sender, Display display, EventContext &context, LayoutContext &layoutContext, Offset &self, Offset &next, bool init)
#define CallDisplayFunc(Fun) Fun(sender, display, context, layoutContext, self, next, false)
#define Layout(display) void display LayoutArgs()
typedef void (*LayoutFunc) LayoutArgs();

Layout(LayoutDisplayNone);
Layout(LayoutDisplayInline);
Layout(LayoutDisplayBlock);
Layout(LayoutDisplayLine);
Layout(LayoutDisplayTable);
Layout(LayoutDisplayRow);

class LayoutManager {
private:
    int m_width = 500;
    int m_height = 500;
    LayoutFunc m_layouts[6] = {
            LayoutDisplayNone,
            LayoutDisplayInline,
            LayoutDisplayBlock,
            LayoutDisplayLine,
            LayoutDisplayTable,
            LayoutDisplayRow,
    };

public:
    void reflow(EventContext context, bool init = false);
    void reflowAll(Document *doc);
    int getHeight() { return m_height; }
    int getWidth() { return m_width; }
};


#endif //GEDITOR_LAYOUT_H
