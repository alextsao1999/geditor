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
};
struct TextLine {
    ColumnNode header;
    GString *getContent(int column) {
        return nullptr;
    }

    void insertColumn(ColumnNode &&col, int column) {

    }

    void deleteColumn(int column = -1) {

    }

    inline ColumnNode **findLastNode() {
        ColumnNode **node = &header.next;
        while ((*node)->next != nullptr)
            node = &(*node)->next;
        return node;
    }

    inline ColumnNode **findNode(int column) {
        ColumnNode **node = &header.next;
        while (column--) {
            if ((*node)->next == nullptr) {

                (*node)->next = new ColumnNode();

            }
            node = &(*node)->next;
        }
        return nullptr;
    }

    ColumnNode *getNode(int column) {
        return nullptr;
    }

};

class LineViewer {
private:
    TextLine *first;
public:
    explicit LineViewer(TextLine *first) : first(first) {}
    inline bool empty() { return first == nullptr; }
    GString *getContent(int column = 0) {
        return nullptr;
    }

};
class TextBuffer {
private:
    std::vector<TextLine> m_buffer;
public:
    int getLineCount() { return m_buffer.size(); }

    void insertLine(int prev = -1) {

    }

    void insertColumn(int line, int prev = -1) {

    }

    void deleteLine(int line) {

    }

    void deleteColumn(int line, int column = 0) {

    }

    void insertString(int pos, int line, int column = 0) {

    }

    void insertChar(int pos, int line, int column = 0) {

    }

    LineViewer getLine(int line) {
        return LineViewer(nullptr);
    }


};



#endif //GEDITOR_TEXT_BUFFER_H
