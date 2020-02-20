//
// Created by Alex on 2019/9/3.
//

#ifndef GEDITOR_CARET_MANAGER_H
#define GEDITOR_CARET_MANAGER_H

#include "paint_manager.h"

class Element;
struct CaretPos {
    int index = 0; // 相对索引
    Offset offset; // 绝对坐标
    CaretPos() = default;
    CaretPos(int index, Offset offset) : index(index), offset(offset) {}
    void setIndex(int idx) {
        index = idx;
        offset = Offset(0, 0);
    }
    void setOffset(Offset value) {
        index = 0;
        offset = value;
    }
    int getIndex() { return index; }
    Offset getOffset() { return offset; }
    void set(int idx, Offset value) {
        index = idx;
        offset = value;
    }
};
class CaretManager {
    friend class RenderManager;
    friend class AutoComplete;
private:
    RenderManager *m_paintManager;
    CaretPos m_data;
public:
    EventContext *m_context = nullptr;
    Offset m_relative;
    explicit CaretManager(RenderManager *paintManager) : m_paintManager(paintManager) {}
    ~CaretManager();
    Element *getFocus();
    EventContext *getEventContext() { return m_context; }
    inline CaretPos &data() { return m_data; }
    // 实际的光标位置
    Offset current();
    void create(int width = 2, int height = 18) {
        CreateCaret(m_paintManager->m_hWnd, nullptr, width, height);
    }
    void destroy() {
        DestroyCaret();
    }
    void show() { ShowCaret(m_paintManager->m_hWnd); }
    void hide() { SetCaretPos(-1, -1); }
    void focus(EventContext *context, bool force = false);
    void set(Offset pos) {
        m_relative = pos;
        update();
    }
    void set(int x, int y) { set(Offset(x, y)); }
    void update();
    bool next();
    bool prev();
    bool enter(int index = 0);
    void leave();
    EventContext *include(Element *element);
    EventContext *include(EventContext *context);
    bool findNext(const GChar *tag);
    bool findPrev(const GChar *tag);
    void onErase(EventContext *context);
};


#endif //GEDITOR_CARET_MANAGER_H
