#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QActionGroup>
#include "cvmatandqimage.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    enum DrawMode { None, Line, Circle };
    Ui::MainWindow *ui;

    void initializeToolBar();
};
#endif // MAINWINDOW_H
