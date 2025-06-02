#ifndef TSURFACEPAINTER_H
#define TSURFACEPAINTER_H

#include <QObject>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsView>

constexpr int PX_DISPLAY_PRESICION = 0;
constexpr int UNITS_MES_DISPLAY_PRESICION = 2;
constexpr int TEXT_DISPLAY_OFFSET_HOR_INPX = 5;
constexpr int TEXT_DISPLAY_OFFSET_VERT_INPX = 5;

class TSurfacePainter : public QObject
{
    Q_OBJECT
public:
    enum class DrawMode : uint { None, Line, Circle };
    explicit TSurfacePainter(QGraphicsScene* scene = nullptr);
public slots:
    void handleMousePressed(QMouseEvent *event);
    void handleMouseMoved(QMouseEvent *event);
    void handleMouseReleased(QMouseEvent *event);
    void clearScene();
    void setFontSize(int size);
    void setLineWidth(int width);
    void setmmInPixelsWidth(double value);
    void setmmInPixelsHeight(double value);
    void setCurrentDrawMode(DrawMode drawMode);
    void setSettingCircleCenter(bool flag);
private:
    QGraphicsScene* scene_;
    DrawMode currentDrawMode_;
    QPointF startPoint_;
    QGraphicsPixmapItem *pixmapItem_;
    QGraphicsItem *tempItem_;
    QGraphicsItem *lastItem_;
    QList<QGraphicsItem*> paintedObjInScene;
    QGraphicsTextItem *tempTextItem_;
    QGraphicsTextItem *lastTextItem_;
    QList<QGraphicsTextItem*> paintedTextObjInScene;
    bool isSettingCircleCenter_ = false;
    bool isDrawing_ = false;
    int fontSize_  = 10;
    int lineWidth_ = 1;
    double mmInPixelsWidth_ = 0.001;
    double mmInPixelsHeight_ = 0.001;

    void clearTempObjs();
    double lineLenghtOfCircleInmm(QPointF centreInPx,QPointF rPointInPx);
    double lineLenghtInmm(QPointF centreInPx,QPointF rPointInPx);
};

#endif // TSURFACEPAINTER_H
