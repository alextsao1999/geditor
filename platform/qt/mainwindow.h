#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCompleter>
#include <geditor.h>
#include <QGEditor.h>
namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
private slots:
    void on_pushButton_clicked();
private:
    Ui::MainWindow *ui;
    QGEditor *editor = nullptr;
    QCompleter *completer = nullptr;

};

#endif // MAINWINDOW_H
