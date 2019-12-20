//
// Created by Alex on 2019/6/27.
//

#ifndef GEDITOR_LAYOUT_H
#define GEDITOR_LAYOUT_H

#include "common.h"

//typedef int Display;
enum Display {
    DisplayNone,
    DisplaySkip,
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
Layout(LayoutDisplaySkip);
Layout(LayoutDisplayInline);
Layout(LayoutDisplayBlock);
Layout(LayoutDisplayLine);
Layout(LayoutDisplayTable);
Layout(LayoutDisplayRow);
Layout(LayoutDisplayCustom);

class LayoutManager {
private:
    uint32_t m_width = 500;
    uint32_t m_height = 2000;
    RenderManager *m_renderManager;
public:
    LayoutManager(RenderManager *renderManager);
    LayoutFunc m_layouts[8] = {
            LayoutDisplayNone,
            LayoutDisplaySkip,
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
    uint32_t getHeight() { return m_height; }
    uint32_t getWidth() { return m_width; }
    LayoutFunc getLayoutFunc(Display display) { return m_layouts[display]; }
};


#endif //GEDITOR_LAYOUT_H
