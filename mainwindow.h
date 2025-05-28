#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QToolBar>
#include <QActionGroup>
#include <QMediaDevices>
#include <QCameraFormat>
 #include <QGraphicsLineItem>
#include "tcameraviewwithpainter.h"

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
    QCamera *camera;
    QMediaCaptureSession *captureSession;
    QGraphicsScene *scene;
    QGraphicsVideoItem *videoItem;
    DrawMode currentDrawMode;
    QPointF startPoint;
    QGraphicsItem *tempItem;
    QGraphicsItem *lastItem;
    QGraphicsTextItem *tempTextItem;
    QGraphicsTextItem *lastTextItem;
    bool isSettingCircleCenter;
    bool isDrawing;
    int fontSize  = 10;

    void initializeToolBar();
    void setupGraphicsView();
    void updateVideoSize();
    void clearScene();
};
#endif // MAINWINDOW_H
