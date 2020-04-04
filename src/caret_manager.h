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
class EventContextRef {
public:
    constexpr EventContextRef() = default;
    EventContextRef(EventContext *ptr) : m_ptr(ptr) {
        if (ptr) {
            m_ref = new int(1);
        }
    }
    ~EventContextRef() { unref(); }
    inline void addref() { if (m_ref) (*m_ref)++; }
    void unref();
    inline EventContext *operator->() { return m_ptr; }
    inline EventContext &operator*() { return *m_ptr; }
    inline EventContextRef &operator=(EventContext *ptr) noexcept {
        if (m_ptr == ptr) {
            return *this;
        }
        unref();
        m_ptr = ptr;
        if (m_ptr != nullptr) {
            m_ref = new int(1);
        }
        return *this;
    }
    inline EventContextRef &operator=(const EventContextRef &rhs) noexcept {
        if (this == &rhs) {
            return *this;
        }
        unref();
        m_ref = rhs.m_ref;
        m_ptr = rhs.m_ptr;
        addref();
        return *this;
    }
    inline bool operator==(EventContext *ptr) noexcept { return m_ptr == ptr; }
    inline bool operator==(const EventContextRef &rhs) noexcept { return m_ptr == rhs.m_ptr; }
    inline EventContext *ptr() { return m_ptr; }
    inline EventContext &ref() { return *m_ptr; }
    inline bool has() { return m_ptr; }
    inline operator bool() { return m_ptr; }
    inline operator EventContext *() { return m_ptr; }
    int *m_ref = nullptr;
    EventContext *m_ptr = nullptr;
};
class CaretManager {
    friend class RenderManager;
    friend class AutoComplete;
private:
    RenderManager *m_paintManager;
    CaretPos m_data;
public:
    EventContextRef m_context;
    Offset m_relative{-10, -10};
    explicit CaretManager(RenderManager *paintManager) : m_paintManager(paintManager) {}
    ~CaretManager();
    EventContextRef &getRef() { return m_context; };
    EventContext *getEventContext() { return m_context.ptr(); }
    inline CaretPos &data() { return m_data; }
    // 可视区光标位置
    Offset current();
/*
    void show() { ShowCaret(m_paintManager->m_hWnd); }
    void hide() { SetCaretPos(-1, -1); }
*/
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
    bool findNext(char *tag);
    bool findPrev(char *tag);
    void onErase(EventContext *context);
    void refocus() {
        focus(m_context.ptr());
    }

};

#endif //GEDITOR_CARET_MANAGER_H
