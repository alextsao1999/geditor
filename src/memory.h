//
// Created by Alex on 2019/7/12.
//
#ifndef GEDITOR_MEMORY_H
#define GEDITOR_MEMORY_H

// 之后根据需要加装内存池...
#include <cstdlib>

#define ge_malloc malloc
#define ge_free free
#define ge_realloc realloc
#define ONCE_ALLOC 4
int CeilToPowerOf2(int v);
template <typename T>
struct Buffer {
    // this is just a buffer
    T *data = nullptr;
    int count = 0;
    int capacity = 0;
    Buffer() = default;
    explicit Buffer(int init_capicity) { ensureCapacity(init_capicity); }
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
    inline void fill(const T &value, int num) {
        ensureCapacity(count + num);
        for (int i = 0; i < num; ++i) {
            data[count++] = value;
        }
    };
    inline int push(const T &data = T()) {
        fill(data, 1);
        return count - 1;
    };
    inline const T &back() { return data[count - 1]; }
    inline T &pop() { return data[--count]; }
    inline void erase(int index) { memcpy(&data[index], &data[index  + 1], (--count - index) * sizeof(T)); };
    inline void erase(int pos, int npos) {
        int n = npos - pos + 1;
        for (int i = 0; i < n; ++i) {
            erase(pos);
        }
    }
    inline bool insert(int index, const T &value = T()) {
        if (index >= count)
            fill(T(), index - count + 1);
        else {
            ensureCapacity(++count);
            memmove(&data[index + 1], &data[index], (count - index - 1) * sizeof(T));
        }
        data[index] = value;
        return true;
    };
    inline void set(int index, const T &value = T()) {
        ensureIndex(index);
        data[index] = value;
    };
    inline T &at(int index) {
        ensureIndex(index);
        return data[index];
    }
    inline int size() {
        return count;
    };
    inline void clear() {
        ge_free(data);
        data = nullptr;
    }
    // 确保容量
    inline void ensureCapacity(int newCapacity) {
        if (newCapacity > capacity) {
            // size_t oldSize = capacity * sizeof(T);
            capacity = CeilToPowerOf2(newCapacity);
            data = (T *) ge_realloc(data, capacity * sizeof(T));
        }
    }
    inline void ensureIndex(int index) {
        if (index >= count)
            fill(T(), index - count + 1);
    }
    inline T &operator[](const int &index) {
        ensureIndex(index);
        return data[index];
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
