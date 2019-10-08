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
#include <iostream>
#define GString std::wstring
template <typename T>
struct Buffer {
    // this is just a buffer
    T *data = nullptr;
    int count = 0;
    int capacity = 0;
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
    inline T pop() { return std::move(data[--count]); }
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

struct ColumnNode {
    GString content;
    ColumnNode *next = nullptr;
    ColumnNode() = default;
    explicit ColumnNode(GString content) : content(std::move(content)) {}
    void free() {
        if (next) {
            next->free();
            delete next;
        }
    }
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
    ColumnNode header;
    TextLine() = default;
    ~TextLine() = default;
    inline void free() {
        header.free();
    }
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
using LineBuffer = std::vector<TextLine>;

class LineViewer {
private:
public:
    int m_line = 0;
    int column = 0;
    LineBuffer *m_buffer = nullptr;
    LineViewer() = default;
    LineViewer(int line, int scolumn, LineBuffer *buffer) : m_line(line), column(scolumn), m_buffer(buffer) {}
    inline int getLineNumber() { return m_line; }
    inline bool empty() { return m_buffer == nullptr; }
    GString &content() {
        return m_buffer->at((unsigned) (m_line)).getNode(column)->content;
    }

    const char *c_str() {
        return (const char *) content().c_str();
    }
    const GChar *str() {
        return (const GChar *) content().c_str();
    }
    GString &string(int col) {
        return m_buffer->at((unsigned) (m_line)).getNode(col)->content;
    }
    void insert(int pos, int ch) {
        auto &str = content();
        str.insert(str.begin() + pos, (GChar) ch);
    }

    void erase(int pos, int length) {
        auto &str = content();
        str.erase(str.begin() + pos, str.begin() + pos + length);
    }
    void remove(int pos) {
        auto &str = content();
        str.erase(str.begin() + pos);
    }
    int length() {
        auto &str = content();
        return str.length();
    }

    size_t size() {
        return length() * sizeof(GChar);
    }

    void append(const GChar *text, int length = 0) {
        auto &str = string(column);
        if (length == 0) {
            length = lstrlen(text);
        }
        str.append(text, length);
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
        m_buffer.insert(m_buffer.begin() + line, TextLine());
        return {line, 0, &m_buffer};
    }
    LineViewer appendLine() {
        m_buffer.emplace_back();
        return {(int) m_buffer.size() - 1, 0, &m_buffer};
    }
    void deleteLine(int line) {
        if (m_buffer.size() == 1) { return; }
        m_buffer[line].free();
        m_buffer.erase(m_buffer.begin() + line);
    }
    LineViewer getLine(int line, int column = 0) {
        if (line >= m_buffer.size()) {
            int rm = line - (int) m_buffer.size() + 1;
            while (rm--) {
                m_buffer.emplace_back();
            }
        }
        return {line, column, &m_buffer};
    }
};

#endif //GEDITOR_TEXT_BUFFER_H
