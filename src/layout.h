//
// Created by Alex on 2019/6/27.
//

#ifndef GEDITOR_LAYOUT_H
#define GEDITOR_LAYOUT_H

#include "common.h"

//typedef int Display;
enum Display {
    DisplayNone,
    DisplayAbsolute,
    DisplayInline,
    DisplayBlock,
    DisplayLine,
    DisplayTable,
    DisplayRow,
    DisplayCustom,
};
class Root;
class RelativeElement;
class Document;
class Element;
class LayoutManager;
class RenderManager;
struct EventContext;
struct Offset;
struct LayoutContext {
    int lineMaxHeight = 0;
    int blockMaxWidth = 0;
};
#define LayoutArgs() EventContext &context, LayoutManager *sender, Display display, LayoutContext &layoutContext, Offset &self, Offset &next, bool relayout
#define UseDisplayFunc(Fun) Fun(context, sender, display, layoutContext, self, next, false)
#define CallDisplayFunc(Fun) Fun(context, sender, display, layoutContext, self, next, relayout)
#define Layout(display) void display (LayoutArgs())
typedef void (*LayoutFunc) (LayoutArgs());

Layout(LayoutDisplayNone);
Layout(LayoutDisplayAbsolute);
Layout(LayoutDisplayInline);
Layout(LayoutDisplayBlock);
Layout(LayoutDisplayLine);
Layout(LayoutDisplayTable);
Layout(LayoutDisplayRow);
Layout(LayoutDisplayCustom);

class LayoutManager {
private:
    RenderManager *m_renderManager;
public:
    explicit LayoutManager(RenderManager *renderManager);
    LayoutFunc m_layouts[8] = {
            LayoutDisplayNone,
            LayoutDisplayAbsolute,
            LayoutDisplayInline,
            LayoutDisplayBlock,
            LayoutDisplayLine,
            LayoutDisplayTable,
            LayoutDisplayRow,
            LayoutDisplayCustom,
    };
    static void ReflowAll(Document *doc);
    void reflow(EventContext context, bool relayout = false, bool outset = false);
    void relayout(EventContext context);
    LayoutFunc getLayoutFunc(Display display) { return m_layouts[display]; }
};

#endif //GEDITOR_LAYOUT_H
