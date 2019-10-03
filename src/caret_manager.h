//
// Created by Alex on 2019/9/3.
//

#ifndef GEDITOR_CARET_MANAGER_H
#define GEDITOR_CARET_MANAGER_H

#include "paint_manager.h"

struct CaretData {
    int index = 0;
    void *data = nullptr;
    CaretData() = default;
    CaretData(int index, void *data) : index(index), data(data) {}
};

class CaretManager {
    friend RenderManager;
private:
    RenderManager *m_paintManager;
    EventContext *m_context = nullptr;
    Offset m_current;
    CaretData m_data;
public:
    explicit CaretManager(RenderManager *paintManager) : m_paintManager(paintManager) {}
    ~CaretManager();
    Element *getFocus ();
    EventContext *getEventContext() { return m_context; }
    void create(int width = 1, int height = 15) {
        CreateCaret(m_paintManager->m_hWnd, nullptr, width, height);
    }
    void destroy() {
        DestroyCaret();
    }
    inline CaretData *data() {
        return &m_data;
    }
    void focus(EventContext *context);
    void show() {
        ShowCaret(m_paintManager->m_hWnd);
    }
    void hide() {
        SetCaretPos(-1, -1);
    }
    void set(Offset pos);
    // 设置相对的光标位置
    void set(int x, int y) {
        set(Offset(x, y));
    }
    void autoSet(int x, int y, int column = 0);
    void update();
    // 当前Focus的EventContext
    bool next();
    bool prev();
    void outerNext();
    void outerPrev();
    bool enter(int index = 0);
    void leave();
};


#endif //GEDITOR_CARET_MANAGER_H
