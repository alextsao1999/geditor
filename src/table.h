//
// Created by Alex on 2019/8/4.
//

#ifndef GEDITOR_TABLE_H
#define GEDITOR_TABLE_H

#include "document.h"
class RowElement : public RelativeElement {

public:
    int column = 0;
    RowElement() = default;
    RowElement(int column) : column(column) {}

    int getLogicWidth(EventContext &context) override {
        return Root::getLogicWidth(context);
    }

    int getLogicHeight(EventContext &context) override {
        return Root::getLogicHeight(context);
    }
};

class TableElement : public RelativeElement {
    int row;
    int column;
    RowElement *cells;
public:
    TableElement(int row, int column) : row(row), column(column) {
        cells = new RowElement[row];
        for (int i = 0; i < row; ++i) {
            cells[i].column = column;
        }
    }

    int getColumnMaxLength(EventContext context, int column) {
        context.getLineViewer().line->getNode(column);
        return 0;
    }

    int getLogicWidth(EventContext &context) override {
        return Root::getLogicWidth(context);
    }

    int getLogicHeight(EventContext &context) override {
        return Root::getLogicHeight(context);
    }
};



#endif //GEDITOR_TABLE_H
