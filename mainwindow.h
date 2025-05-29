#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <QCameraDevice>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QMediaDevices>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QVideoFrame>
#include <QToolBar>
#include <QActionGroup>
#include <QTimer>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
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
    QVideoSink *videoSink;
    DrawMode currentDrawMode;
    QPointF startPoint;
    QGraphicsPixmapItem *pixmapItem;
    QGraphicsItem *tempItem;
    QGraphicsItem *lastItem;
    QGraphicsTextItem *tempTextItem;
    QGraphicsTextItem *lastTextItem;
    cv::VideoCapture *rtspCapture = nullptr;
    QTimer *rtspTimer = nullptr;
    bool isSettingCircleCenter;
    bool isDrawing;
    int fontSize  = 10;
    bool enableCanny = false;

    void initializeToolBar();
    void setupGraphicsView();
    void updateVideoSize();
    void clearScene();
private slots:
    void processFrame(const QVideoFrame &frame);
    void updateCannyState(int state);
    void startRtspCapture(const QString &url);
    void stopRtspCapture();
    void processRtspFrame();
};
#endif // MAINWINDOW_H
