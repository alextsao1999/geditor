//
// Created by Administrator on 2019/6/29.
//

#ifndef GEDITOR_TEXT_BUFFER_H
#define GEDITOR_TEXT_BUFFER_H

#include "common.h"
#include <string>
#include <vector>
enum class NodeType {
    Original,
    Add,
    Delete
};
struct Node {
    NodeType type;
    int column;
    int start;
    int length;
};
class PieceTable {
public:
private:
    GString m_original;
    GString m_added;
    std::vector<Node> m_nodes;
};


class TextBufferManager {
private:
    PieceTable m_table;
public:

    void insertNewLine(int prev) {

    }
    int getLineCount() {
        return 0;
    }
    GString getLineContent(int lineNum) {
        return GString();
    }


};



#endif //GEDITOR_TEXT_BUFFER_H
