//
// Created by Alex on 2019/7/12.
//
#ifndef GEDITOR_MEMORY_H
#define GEDITOR_MEMORY_H

// 之后根据需要加装内存池...
#include <cstdlib>
#include <utility>
#define ge_malloc malloc
#define ge_free free
#define ge_realloc realloc
#define ge_new(element, ...) (new element(__VA_ARGS__))
#define ge_delete(element) (delete element)

#define ONCE_ALLOC 4
int CeilToPowerOf2(int v);
template <typename T>
class Buffer {
    // this is just a buffer
private:
    T *m_data = nullptr;
    int m_count = 0;
    int m_capacity = 0;
public:
    Buffer() = default;
    explicit Buffer(int init_count) { m_count = init_count;ensureCapacity(init_count); }
    ~Buffer() { clear(); }
    class BufferIter {
        Buffer *m_data;
        int m_index = 0;
    public:
        explicit BufferIter(Buffer *data) : m_data(data) {}
        inline bool has() { return m_index < m_data->size(); }
        inline void next() { m_index++; }
        inline int index() { return m_index; }
        inline T &current() { return m_data->operator[](m_index); }
    };
    inline BufferIter iter() { return BufferIter(this); }
    inline T *begin() { return m_data; }
    inline T *end() { return m_data + m_count; }
    inline T *data() { return m_data; }
    inline int size() { return m_count; }
    inline int length() { return m_count; }
    inline void fill(const T &value, int num) {
        ensureCapacity(m_count + num);
        for (int i = 0; i < num; ++i) {
            m_data[m_count++] = value;
        }
    };
    inline int push(const T &data = T()) {
        fill(data, 1);
        return m_count - 1;
    };
    inline T &front() { return m_data[0]; }
    inline const T &back() { return m_data[m_count - 1]; }
    inline T &pop() { return m_data[--m_count]; }
    inline void erase(int index) { memcpy(&m_data[index], &m_data[index + 1], (--m_count - index) * sizeof(T)); };
    inline void erase(int pos, int npos) {
        int n = npos - pos + 1;
        for (int i = 0; i < n; ++i) {
            erase(pos);
        }
    }
    inline bool insert(int index, const T &value = T()) {
        if (index >= m_count)
            fill(value, index - m_count + 1);
        else {
            ensureCapacity(++m_count);
            memmove(&m_data[index + 1], &m_data[index], (m_count - index - 1) * sizeof(T));
        }
        m_data[index] = value;
        return true;
    };
    inline void set(int index, const T &value = T()) {
        ensureIndex(index);
        m_data[index] = value;
    };
    inline T &at(int index) {
        ensureIndex(index);
        return m_data[index];
    }
    inline void clear() {
        ge_free(m_data);
        m_data = nullptr;
    }
    inline void resize(int new_count = 0) { m_count = new_count;ensureCapacity(new_count); }
    // 确保容量
    inline void ensureCapacity(int newCapacity) {
        if (newCapacity > m_capacity) {
            // size_t oldSize = capacity * sizeof(T);
            m_capacity = CeilToPowerOf2(newCapacity);
            m_data = (T *) ge_realloc(m_data, m_capacity * sizeof(T));
        }
    }
    inline void ensureIndex(int index) {
        if (index >= m_count) fill(T(), index - m_count + 1);
    }
    inline T &operator[](const int &index) {
        ensureIndex(index);
        return m_data[index];
    }

};

template <typename Type>
class CachedList {
    struct Node {
        Type *ptr = nullptr;
        Node *next = nullptr;
    };
    Node *frees = nullptr;
    Node *unused = nullptr;
    int refer = 0;
    int alloc = 0;
    Buffer<Type *> buffer;

    Node *remove(Node **node, Type *ptr) {
        if (*node == nullptr) {
            return nullptr;
        } else {
            do {
                if ((*node)->ptr == ptr) {
                    Node *removed = *node;
                    *node = (*node)->next;
                    return removed;
                }
            } while (*(node = &(node->next)) != nullptr);
        }
        return nullptr;
    }

};

template <typename Type>
class Cached {
public:
    static CachedList<Type> List;
    static Type *alloc() {
        return nullptr;
    }
    static void free(Type *ptr) {
    }
};

template<typename Type>
CachedList<Type> Cached<Type>::List;
#endif //GEDITOR_MEMORY_H
