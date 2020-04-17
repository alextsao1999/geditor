//
// Created by Alex on 2020/1/29.
//

#include "auto_complete.h"
#include "event.h"
#include "document.h"
void AutoComplete::show(Document *document) {
    //auto &caret = document->context()->m_caretManager;
    RECT rect;
    ShowWindow(m_hWnd, SW_SHOW);
    SetFocus(m_hWnd);
/*
    GetWindowRect(caret.m_paintManager->m_hWnd, &rect);
    Offset offset = caret.current();
    offset.y += caret.getEventContext()->height() - 5;
    MoveWindow(m_hWnd, rect.left + offset.x, rect.top + offset.y, 250, 300, true);
    ShowWindow(m_hWnd, SW_SHOW);
    SetFocus(caret.m_paintManager->m_hWnd);
*/

}
class CompleteItem : public RelativeElement {
public:
    int getLogicWidth(EventContext &context) override {
        return 200;
    }

    int getLogicHeight(EventContext &context) override {
        return 50;
    }

    void onRedraw(EventContext &context) override {
        Canvas canvas = context.getCanvas();
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorBLACK);
        canvas->drawRect(context.bound(), paint);
        printf("redraw!\n");
    }
};
class CompleteList : public Container<> {
public:
    CompleteList() {
        Container::append(new CompleteItem());
        Container::append(new CompleteItem());
        Container::append(new CompleteItem());
        Container::append(new CompleteItem());
    }

    void onRedraw(EventContext &context) override {
        printf("list redraw\n");
        Root::onRedraw(context);
    }

};
CompleteDocument::CompleteDocument(RenderManager *render, DocumentManager *mgr) :
Document(render, mgr), m_context(render) {
    Container::append(new CompleteList());
    root().relayout();
    //m_root.redraw();
}

void CompleteDocument::onRedraw(EventContext &context) {
    printf("redraw document\n");
    Root::onRedraw(context);
}
