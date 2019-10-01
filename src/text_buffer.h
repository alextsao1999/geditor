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
    LineBuffer *m_buffer = nullptr;
    LineViewer() = default;
    LineViewer(int number, LineBuffer *buffer) : m_line(number), m_buffer(buffer) {}
    inline int getLineNumber() { return m_line; }
    inline bool empty() { return m_buffer == nullptr; }
    GString &content(int column = 0) {
        return m_buffer->at((unsigned) (m_line)).getNode(column)->content;
    }

    const char *c_str(int column = 0) {
        return (const char *) string(column).c_str();
    }
    const GChar *str(int column = 0) {
        return (const GChar *) string(column).c_str();
    }
    GString &string(int column = 0) {
        return m_buffer->at((unsigned) (m_line)).getNode(column)->content;
    }
    void insert(int pos, int ch, int column = 0) {
        auto &str = string(column);
        str.insert(str.begin() + pos, (GChar) ch);
    }
    void erase(int pos, int length, int column = 0) {
        auto &str = string(column);
        str.erase(str.begin() + pos, str.begin() + pos + length);
    }
    void remove(int pos, int column = 0) {
        auto &str = string(column);
        str.erase(str.begin() + pos);
    }
    int length(int column = 0) {
        auto &str = string(column);
        return str.length();
    }
    int size(int column = 0) {
        return length(column) * 2;
    }

    void append(const GChar *sstr, int length = 0, int column = 0) {
        auto &str = string(column);
        if (length == 0) {
            length = lstrlen(sstr);
        }
        str.append(sstr, length);
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
        return {line, &m_buffer};
    }
    LineViewer appendLine() {
        m_buffer.emplace_back();
        return {(int) m_buffer.size() - 1, &m_buffer};
    }
    void deleteLine(int line) {
        if (m_buffer.size() == 1) { return; }
        m_buffer[line].free();
        m_buffer.erase(m_buffer.begin() + line);
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
