//
// Created by Administrator on 2019/9/3.
//

#ifndef GEDITOR_CARET_MANAGER_H
#define GEDITOR_CARET_MANAGER_H

#include "paint_manager.h"

class CaretManager {
    friend PaintManager;
private:
    PaintManager *m_paintManager;
    EventContext *m_context = nullptr;
    Element *m_focus = nullptr;
    Offset m_current;

public:
    explicit CaretManager(PaintManager *paintManager) : m_paintManager(paintManager) {}
    ~CaretManager();
    Element *getFocus () {
        return m_focus;
    }
    EventContext *getEventContext() { return m_context; }
    void create() {
        CreateCaret(m_paintManager->m_hWnd, nullptr, 2, 15);
    }
    void focus(EventContext *context);
    void show() {
        ShowCaret(m_paintManager->m_hWnd);
    }
    void hide() {
        SetCaretPos(-1, -1);
    }
    void set(Offset pos) {
        m_current = pos;
        update();
    }
    void set(int x, int y) {
        m_current.x = x;
        m_current.y = y;
        update();
    }
    Offset get() { return m_current; }
    void update();

};


#endif //GEDITOR_CARET_MANAGER_H
