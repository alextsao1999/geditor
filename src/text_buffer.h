//
// Created by Alex on 2019/6/29.
//  先用ArrayMethod 看看效果 之后试试加成内存池和PieceTable
// Line都是index
//

#ifndef GEDITOR_TEXT_BUFFER_H
#define GEDITOR_TEXT_BUFFER_H

#include "common.h"
#include "memory.h"
#include <string>
#include <vector>
#include <list>
/*
class GString {
public:
    GChar *string = nullptr;
    uint32_t length = 0;
    GString() = default;
    ~GString() = default;

    const GChar *c_str() {
        return string;
    }
    inline uint32_t size() { return length; }
    void append(const GChar *str) {

    }

    void append(const GString &str) {

    }
    void erase(int index) {

    }
    void erase(int index, int n) {

    }
    void insert(int index, GChar ch) {

    }

    GString substr(int pos, int n) {
        return {};

    }
};
*/

struct ColumnNode {
    GString content;
    ColumnNode *next = nullptr;
    ColumnNode() = default;
    explicit ColumnNode(const GString &content) : content(content) {}
};

struct TextLine {
    static void FreeAll(ColumnNode *node) {
        if (node == nullptr)
            return;
        FreeAll(node->next);
        delete node;
    };
    ColumnNode header;
    ~TextLine() { FreeAll(header.next); }
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

class LineViewer {
private:
public:
    int m_line = 0;
    std::vector<TextLine> *m_buffer = nullptr;
    LineViewer() = default;
    LineViewer(int number, std::vector<TextLine> * buffer) : m_line(number), m_buffer(buffer) {}
    inline int getLineNumber() { return m_line; }
    inline bool empty() { return m_buffer == nullptr; }
    GString &content(int column = 0) {
        return m_buffer->at((unsigned) (m_line)).getNode(column)->content;
    }

};

class TextBuffer {
private:
    std::vector<TextLine> m_buffer;
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
