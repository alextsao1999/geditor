//
// Created by Alex on 2019/6/29.
//  先用ArrayMethod 看看效果 之后试试加成内存池和PieceTable
// Line都是index
//

#ifndef GEDITOR_TEXT_BUFFER_H
#define GEDITOR_TEXT_BUFFER_H

#include "common.h"
#include <string>
#include <vector>

struct ColumnNode {
    GString content;
    ColumnNode *next = nullptr;
    ColumnNode() = default;
    explicit ColumnNode(GString content) : content(std::move(content)) {}
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
    void insertColumn(ColumnNode *col, int column = -1) {
        if (column == 0)
            return;
        ColumnNode **node;
        ColumnNode *next = nullptr;
        if (column < 0) {
            node = findLastNode();
        } else {
            node = findNode(column);
            next = *node;
        }
        col->next = next;
        *node = col;
    }
    void deleteColumn(int column = -1) {
        if (column == 0)
            return;
        ColumnNode **node;
        if (column < 0)
            node = findLastNode();
        else
            node = findNode(column);
        if (*node == nullptr) {
            return;
        }
        ColumnNode *next = (*node)->next;
        delete *node;
        *node = next;
    }
    inline ColumnNode **findLastNode() {
        ColumnNode **node = &header.next;
        while (*node != nullptr)
            node = &(*node)->next;
        return node;
    }
    inline ColumnNode **findNode(int column) {
        ASSERT(column > 0, "column must greater than 0!");
        ColumnNode **node = &header.next;
        while (--column) {
            if (*node == nullptr)
                *node = new ColumnNode();
            node = &(*node)->next;
        }
        return node;
    }
    ColumnNode *getNode(int column) {
        if (!column)
            return &header;
        return *findNode(column);
    }
};

class LineViewer {
private:
public:
    int number;
    TextLine *line;
    LineViewer(int number, TextLine *line) : number(number), line(line) {}
    inline bool empty() { return line == nullptr; }
    GString &getContent(int column = 0) {
        return line->getNode(column)->content;
    }
    inline int getLineNumber() { return number; }

};

class TextBuffer {
private:
    std::vector<TextLine> m_buffer;
public:
    TextBuffer() = default;
    inline int getLineCount() { return m_buffer.size(); }
    LineViewer insertLine(int prev = -1) {
        m_buffer.insert(m_buffer.begin() + prev, TextLine());
        return {prev, &m_buffer[prev]};
    }
    LineViewer appendLine() {
        m_buffer.emplace_back();
        return {(int) (m_buffer.size() - 1), &m_buffer.back()};
    }

    void insertColumn(int line, int prev = -1) {
        m_buffer[line].insertColumn(new ColumnNode(), prev);
    }

    void deleteLine(int line) {
        if (m_buffer.size() == 1) { return; }
        m_buffer.erase(m_buffer.begin() + line);
    }

    void deleteColumn(int line, int column = -1) {
        m_buffer[line].deleteColumn(column);
    }

    LineViewer getLine(int line) {
        return {line, &m_buffer[line]};
    }

};

#endif //GEDITOR_TEXT_BUFFER_H
