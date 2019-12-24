//
// Created by Alex on 2019/6/29.
//  先用ArrayMethod 看看效果 之后试试加成内存池和PieceTable
//

#ifndef GEDITOR_TEXT_BUFFER_H
#define GEDITOR_TEXT_BUFFER_H

#include "common.h"
#include "line_index.h"
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
    int column = 0;
    LineBuffer *m_buffer = nullptr;
    LineViewer() = default;
    LineViewer(int line, int scolumn, LineBuffer *buffer) : m_line(line), column(scolumn), m_buffer(buffer) {}
    inline int getLineNumber() { return m_line; }
    inline bool empty() { return m_buffer == nullptr; }
    GString &content() {
        return m_buffer->at((unsigned) (m_line)).getNode(column)->content;
    }
    const GChar *c_str() {
        return (const GChar *) content().c_str();
    }
    GString &string(int col) {
        return m_buffer->at((unsigned) (m_line)).getNode(col)->content;
    }
    void insert(int pos, int ch) {
        auto &str = content();
        str.insert(str.begin() + pos, (GChar) ch);
    }
    void remove(int pos, int length) {
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
        if (m_buffer.size() == 1 || line == 0) { return; }
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

    LineViewer insertLine(LineCounter counter, int offset) {
        if (counter.line >= m_buffer.size()) {
            return getLine(counter.line);
        }
        m_buffer.insert(m_buffer.begin() + counter.line + offset, TextLine());
        return {counter.line + 1, 0, &m_buffer};
    }
    void deleteLine(LineCounter counter) {
        if (m_buffer.size() == 1 || counter.line == 0) { return; }
        m_buffer[counter.line].free();
        m_buffer.erase(m_buffer.begin() + counter.line);
    }
    LineViewer getLine(LineCounter counter, int offset, int column = 0) {
        int line = counter.line + offset;
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
