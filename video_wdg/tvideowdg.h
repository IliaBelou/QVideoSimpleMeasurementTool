#ifndef TVIDEOWDG_H
#define TVIDEOWDG_H

#include <QGraphicsView>
#include <QObject>
#include <QTimer>
#include <QPixmap>

#include "video_wdg/surface_painter/tsurfacepainter.h"
#include "video_wdg/frame_middleware/iframemiddleware.h"
#include "video_wdg/frame_providers/iframeprovider.h"

constexpr int FRAME_UPDATE_PERIOD = 50; // in ms
constexpr double ZOOM_FACTOR = 0.05;

class TVideoWdg : public QGraphicsView
{
    Q_OBJECT
public:
    TVideoWdg(QWidget *parent = nullptr);
    ~TVideoWdg();
    TSurfacePainter* getPainter();
signals:
    void mousePressed(QMouseEvent *event);
    void mouseMoved(QMouseEvent *event);
    void mouseReleased(QMouseEvent *event);
    void resized();
    void videoSourcesChanged(QList<std::string> srcs);
    void videoFormatsChanged(QList<std::string> fmts); 
public slots:
    void updateFrame();
    void changeVideoSrc(const QString& src);
    void changeVideofmt(int idx);
    void incZoom();
    void decZoom();
    void fit();
    void useEdgeDetector(bool use);
    void addRTCPsource(QString url);
protected:
    void mousePressEvent(QMouseEvent *event) override { emit mousePressed(event); }
    void mouseMoveEvent(QMouseEvent *event) override { emit mouseMoved(event); }
    void mouseReleaseEvent(QMouseEvent *event) override { emit mouseReleased(event); }
    void resizeEvent(QResizeEvent *event) override {
        QGraphicsView::resizeEvent(event);
        emit resized();
    }
private:
    QGraphicsScene *scene_;
    TSurfacePainter *painter_;
    QList<IFrameProvider* > fproviders_;
    std::vector<std::unique_ptr<IFrameMiddleware> > fmiddlewares_;
    QTimer* updateFrame_;
    QGraphicsPixmapItem *currentFrame_;
    QGraphicsPixmapItem *prevcurrentFrame_;
    void updateVideoSize(const QImage &img);
    int currentActiveVideoProviderIdx_ = 0;
    std::string currentActiveFormatSrc_{};
    QList<std::string> videosrcDesc_;
    QList<std::string> videofmtDesc_;
    IFrameProvider* usbDevs_;
    IFrameProvider* rtcp_;
    double zoomFactor_ = 1.0;
    QImage currentFrameImg_;

    void addMiddleware(IFrameMiddleware *middleware);
    template<typename T>
    void removeMiddlewareByType();
    void removeAllEdgeDetectors();
    template<typename T>
    bool findMiddlewareByType();
};

#endif // TVIDEOWDG_H
