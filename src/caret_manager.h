//
// Created by Alex on 2019/9/3.
//

#ifndef GEDITOR_CARET_MANAGER_H
#define GEDITOR_CARET_MANAGER_H

#include "paint_manager.h"
class Element;
struct CaretPos {
    int index = 0;
    Offset offset;
    void *data = nullptr;
    CaretPos() = default;
    void setIndex(int idx) {
        index = idx;
        offset = Offset(0, 0);
    }
    void setOffset(Offset value) {
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
    friend RenderManager;
private:
    RenderManager *m_paintManager;
    EventContext *m_context = nullptr;
    CaretPos m_data;
public:
    Offset m_current;
    CaretManager(RenderManager *paintManager) : m_paintManager(paintManager) {}
    ~CaretManager();
    Element *getFocus();
    EventContext *getEventContext() { return m_context; }
    inline CaretPos &data() { return m_data; }

    bool direction = false;
    int counter = 0;
    void onFrame() {
        counter = direction ? counter - 15 : counter + 15;
        if (counter >= 240 || counter <= 0) {
            direction = !direction;
        }
        SkPaint paint;
        paint.setStyle(SkPaint::Style::kStroke_Style);
        paint.setStrokeWidth(2);
        paint.setColor(SK_ColorBLACK);
        paint.setAlpha(counter);
        Offset offset = current() - m_paintManager->getViewportOffset();
        m_paintManager->m_canvas->drawLine(offset.x, offset.y, offset.x, offset.y + 20, paint);

    }

    Offset current();
    void create(int width = 1, int height = 18) {
        CreateCaret(m_paintManager->m_hWnd, nullptr, width, height);
    }
    void destroy() {
        DestroyCaret();
    }
    void show() { ShowCaret(m_paintManager->m_hWnd); }
    void hide() { SetCaretPos(-1, -1); }
    void focus(EventContext *context);
    void set(Offset pos) {
        m_current = pos;
        update();
    }
    // 设置相对的光标位置
    void set(int x, int y) { set(Offset(x, y)); }
    void update();
    // 当前Focus的EventContext
    bool next();
    bool prev();
    bool enter(int index = 0);
    void leave();

    bool findNext(const GChar *tag);
    bool findPrev(const GChar *tag);

};


#endif //GEDITOR_CARET_MANAGER_H
