#include <utility>

//
// Created by Alex on 2019/6/29.
//  先用ArrayMethod 看看效果 之后试试加成内存池和PieceTable
// Line都是index
//

#ifndef GEDITOR_TEXT_BUFFER_H
#define GEDITOR_TEXT_BUFFER_H

#include "common.h"
#include "memory.h"
#include <vector>
#include <list>
#include <iostream>
#include <string.h>
#define BUFFER_GROWTH(count) CeilToPowerOf2(count)

template<typename T>
T *MemManager(T *ptr, size_t newSize) {
    if (newSize == 0){
        ge_free(ptr);
        return nullptr; // 防止野指针
    }
    return (T *) ge_realloc(ptr, newSize);
}

template <typename T>
struct Buffer {
    T *data = nullptr;
    int count = 0;
    int capacity = 0;
    template<typename ...Args>
    T &emplace_back(Args &&...args) {
        fill(T(std::forward<Args>(args)...), 1);
        return data[count];
    }
    inline void fill(const T &value, int num) {
        ensureCapacity(count + num);
        for (int i = 0; i < num; ++i) {
            data[count++] = value;
        }
    };
    inline int push(const T &data) {
        fill(data, 1);
        return count - 1;
    };
    inline const T &back() {
        return data[count - 1];
    }
    inline const T &pop() {
        return data[--count];
    }
    // 调用析构函数
    inline void erase(int index) {
        data[index].~T();
        memcpy(&data[index], &data[index  + 1], (--count - index) * sizeof(T));
    };
    inline void erase(int pos, int npos) {
        int n = npos - pos + 1;
        for (int i = 0; i < n; ++i) {
            erase(pos);
        }
    }
    inline bool insert(int index, const T &value) {
        if (index >= count)
            fill(T(), index - count + 1);
        else {
            ensureCapacity(++count);
            memmove(&data[index + 1], &data[index], (count - index - 1) * sizeof(T));
        }
        data[index] = value;
        return true;
    };
    inline void set(int index, const T &value) {
        if (index >= count)
            fill(T(), index - count + 1);
        data[index] = value;
    };
    inline T &at(int index) {
        if (index >= count)
            fill(T(), index - count + 1);
        return data[index];
    }
    inline int size() {
        return capacity * sizeof(T);
    };
    inline void clear() {
        data = (T *) MemManager(data, 0);
    }
    // 确保容量
    inline void ensureCapacity(int newCapacity) {
        if (newCapacity > capacity) {
            capacity = BUFFER_GROWTH(newCapacity);
            data = MemManager(data, capacity * sizeof(T));
        }
    }
    // 缩小到合适大小
    void shrink() {
        if (capacity > count) {
            data = (T *) MemManager(data, 0);
            capacity = count;
        }
    }
    inline T &operator[](const int &index) {
        if (index >= count)
            fill(T(), index - count + 1);
        return data[index];
    }
};

class GString {
public:
    Buffer<GChar> string;
    GString() = default;
    ~GString() = default;
    const GChar *c_str() const {
        string.data[string.count + 1] = _GT('\0');
        return string.data;
    }
    inline int size() { return string.count; }
    int begin() { return 0; }
    int end() { return 0; }
    void append(const GChar *str) {
        append(str, lstrlen(str));
    }
    void append(const GChar *str, int len) {
        int before = string.count;
        string.count = before + len;
        string.ensureCapacity(string.count + 1);
        memcpy(string.data + before, str, (size_t) len);
        string.data[string.count + 1] = _GT('\0');
    }
    void append(const GString &str) {
        append(str.c_str());
    }
    void erase(int index) {
        string.erase(index);
    }
    void erase(int index, int n) {
        string.erase(index, n);
    }
    void insert(int index, GChar ch) {
        string.insert(index, ch);
    }
    GString substr(int pos, int npos) {
        GString str;
        str.append(string.data + pos, npos - pos + 1);
        return str;
    }
    friend std::wostream &operator<<(std::wostream &os, const GString &string) {
        os << (const GChar *) string.string.data;
        return os;
    }

};

struct ColumnNode {
    GString content;
    ColumnNode *next = nullptr;
    ColumnNode() = default;
    explicit ColumnNode(GString content) : content(std::move(content)) {}
    friend std::ostream &operator<<(std::ostream &os, const ColumnNode &node) {
        os << "content: " << node.content.c_str();
        if (node.next) {
            os << " | " << *(node.next);
        }
        os << std::endl;
        return os;
    }
};

struct TextLine {
    static void FreeAll(ColumnNode *node) {
        if (node == nullptr)
            return;
        FreeAll(node->next);
        delete node;
    };
    ColumnNode header;
    TextLine() = default;
    ~TextLine() { /*FreeAll(header.next);*/ }
    inline ColumnNode *findNode(int column) {
        ColumnNode *node = &header;
        while (column--) {
            if (node->next == nullptr)
                node->next = new ColumnNode();
            node = node->next;
        }
        return node;
    }
    ColumnNode *getNode(int column) {
        if (!column)
            return &header;
        return findNode(column);
    }
};
using LineBuffer = Buffer<TextLine>;

class LineViewer {
private:
public:
    int m_line = 0;
    LineBuffer *m_buffer = nullptr;
    LineViewer() = default;
    LineViewer(int number, LineBuffer * buffer) : m_line(number), m_buffer(buffer) {}
    inline int getLineNumber() { return m_line; }
    inline bool empty() { return m_buffer == nullptr; }
    GString &content(int column = 0) {
        return m_buffer->at((unsigned) (m_line)).getNode(column)->content;
    }

};

class TextBuffer {
private:
    LineBuffer m_buffer;
public:
    TextBuffer() = default;
    inline int getLineCount() { return m_buffer.size(); }
    LineViewer insertLine(int line) {
        if (line >= m_buffer.size()) {
            return getLine(line);
        }
        m_buffer.insert(line, TextLine());
        return {line, &m_buffer};
    }
    LineViewer appendLine() {
        m_buffer.fill(TextLine(), 1);
        return {(int) m_buffer.size() - 1, &m_buffer};
    }
    void deleteLine(int line) {
        if (m_buffer.size() == 1) { return; }
        m_buffer.erase(line);
    }
    LineViewer getLine(int line) {
        if (line >= m_buffer.size()) {
            int rm = line - m_buffer.size() + 1;
            while (rm--) {
                m_buffer.emplace_back();
            }
        }
        return {line, &m_buffer};
    }
};

#endif //GEDITOR_TEXT_BUFFER_H
