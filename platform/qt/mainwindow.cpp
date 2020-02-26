#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QtGui/QStandardItemModel>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    editor = GEditorBuilder::build((HWND) (this->winId()), 10, 10);

    QStringList list = {"aa", "bb", "ccc"};
    completer = new QCompleter(list, this);

}

MainWindow::~MainWindow() {
    delete ui;
    delete editor;
}

void MainWindow::on_pushButton_clicked() {
    if (auto *view = completer->popup()) {
        auto pt = QCursor::pos();
        view->move(pt);
        view->show();
    }
    //QMessageBox::information(NULL, "Title", "Content", QMessageBox::Yes);

}
