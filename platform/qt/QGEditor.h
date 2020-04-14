//
// Created by Alex on 2020/2/24.
//

#ifndef GEDITOR_QGEDITOR_H
#define GEDITOR_QGEDITOR_H

#include <QWidget>
#include <QPainter>
#include <QBitmap>
#include <memory>
#include <SkSurface.h>
#include "common.h"
#include "utils.h"
#include "paint_manager.h"
#include "table.h"
#include "doc_manager.h"
class QtRender : public RenderManager {
public:
    QWidget *m_widget;
    DocumentManager m_manager;
    QImage m_image;
    SkBitmap m_bitmap;
    std::shared_ptr<SkCanvas> m_canvas;
    QtRender(QWidget *widget) : m_manager(this), m_widget(widget) {}
    void refresh() override {}
    void invalidate() override {
        m_widget->repaint();
    }
    void update(GRect *rect) override {
        if (m_manager.m_documents.empty()) {
            return;
        }
        auto *current = m_manager.current();
        current->onRedraw(current->m_root);
    }
    void resize() override {
        int width = m_widget->width(), height = m_widget->height();
        SkImageInfo info;
        info = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        m_bitmap.allocPixels(info);
        //m_image = QImage((uchar *) m_bitmap.getPixels(), width, height, QImage::Format_RGBA8888_Premultiplied);
        m_canvas = std::make_shared<SkCanvas>(m_bitmap);
        m_canvas->clear(SK_ColorWHITE);
        return;
        m_image = QImage(width, height, QImage::Format_RGBA8888_Premultiplied);
        m_bitmap.installPixels(info, m_image.bits(), m_image.bytesPerLine());
        m_canvas = std::make_shared<SkCanvas>(m_bitmap);
    }
    Canvas getCanvas(EventContext *ctx) override {
        return Canvas(ctx, m_canvas.get());
    }
    void setViewportOffset(Offset offset) override {
        auto *current = m_manager.current();
        current->setViewportOffset(offset);
    }
    void setVertScroll(uint32_t height) override {
    }
    Size getViewportSize() override {
        return {m_widget->width(), m_widget->height()};
    }
    bool copy() override {
        QPainter painter(m_widget);
        painter.drawImage(0, 0, m_image);
        return true;
    }

};
class QGEditor : public QWidget {
    Q_OBJECT
public:
    QtRender m_render;
    explicit QGEditor(QWidget *parent) : QWidget(parent, Qt::WindowType::Widget), m_render(this) {
        setFixedSize(850, 500);
        m_render.resize();
    }
protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setBrush(QBrush(Qt::white));
        painter.drawRect(rect());
        m_render.update(nullptr);
        m_render.copy();
    }
};


#endif //GEDITOR_QGEDITOR_H
