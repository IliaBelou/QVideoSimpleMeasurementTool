#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QToolBar>
#include <QActionGroup>
#include <QMediaDevices>
#include <QCameraFormat>
 #include <QGraphicsLineItem>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <cmath>
#include "tcameraviewwithpainter.h"
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
    QCamera *camera;
    QMediaCaptureSession *captureSession;
    QGraphicsScene *scene;
    QGraphicsVideoItem *videoItem;
    QVideoSink *videoSink;
    DrawMode currentDrawMode;
    QPointF startPoint;
    QGraphicsPixmapItem *pixmapItem;
    QGraphicsItem *tempItem;
    QGraphicsItem *lastItem;
    QGraphicsTextItem *tempTextItem;
    QGraphicsTextItem *lastTextItem;
    bool isSettingCircleCenter;
    bool isDrawing;
    int fontSize  = 10;
    bool enableCanny = false;

    void initializeToolBar();
    void setupGraphicsView();
    void updateVideoSize();
    void clearScene();
    QImage Mat2QImage(const cv::Mat &mat);
    cv::Mat QImage2Mat(const QImage &image);
private slots:
    void processFrame(const QVideoFrame &frame);
    void updateCannyState(int state);
};
#endif // MAINWINDOW_H
