//
// Created by Alex on 2020/2/24.
//

#ifndef GEDITOR_QGEDITOR_H
#define GEDITOR_QGEDITOR_H

#include <QWidget>
#include <QPainter>
#include <QBitmap>
#include "geditor.h"
class QGEditor : public QWidget {
    Q_OBJECT
public:
    explicit QGEditor(QWidget *parent) : QWidget(parent, Qt::WindowType::Widget) {}
protected:
    void paintEvent(QPaintEvent *event) override {
        QBitmap bitmap;
        QPainter painter(&bitmap);
        QPen pen;

    }
};


#endif //GEDITOR_QGEDITOR_H
