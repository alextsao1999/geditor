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
    EventContextRef(EventContextRef &rhs) {
        m_ptr = rhs.m_ptr;
        m_ref = rhs.m_ref;
        addref();
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
struct CaretContext{
    Offset relative;
    EventContextRef context;
    CaretPos data;
};
class CaretManager {
    friend class RenderManager;
    friend class AutoComplete;
private:
    RenderManager *m_paintManager;
public:
    EventContextRef m_context;
    CaretPos m_data;
    Offset m_relative{-10, -10};
    explicit CaretManager(RenderManager *paintManager) : m_paintManager(paintManager) {}
    ~CaretManager();
    EventContextRef &getRef() { return m_context; };
    EventContext *getEventContext() { return m_context.ptr(); }
    inline CaretPos &data(EventContext *sender = nullptr) { return m_data; }
    // 可视区光标位置
    Offset current();
    void focus(EventContext *sender, EventContext *context, bool force = false);
    void set(Offset pos, int index = 0);
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
    void refocus() {
        focus(nullptr, m_context.ptr());
    }
public:
    bool direct = true;
    bool hide = false;
    double process = 0;
    int alpha = 0;
    void onErase(EventContext *context);
    void onTick();
    void onDraw();
    void onShow();
    void onHide() {
        process = 0;
        onTick();
        hide = true;
    }
    static inline double EaseInOutQuint(double t) {
        double t2;
        if( t < 0.5 ) {
            t2 = t * t;
            return 16 * t * t2 * t2;
        } else {
            t2 = (--t) * t;
            return 1 + 16 * t * t2 * t2;
        }
    }

};

#endif //GEDITOR_CARET_MANAGER_H
