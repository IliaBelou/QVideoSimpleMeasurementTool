#ifndef TVIDEOWDG_H
#define TVIDEOWDG_H

#include <QObject>
#include <QVideoWidget>
#include <QPoint>
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>
#include <QDebug>
#include <QTimer>

class TLinePainter : public QWidget {
    Q_OBJECT
public:
    TLinePainter(QWidget *parent = nullptr);
    enum class LineType : uint
    {
        LINE = 1,
        ARC = 2
    };
    void handlePressEvent(QMouseEvent *event) {
        qDebug() << "leftpressPainter";
        if (event->button() == Qt::LeftButton) {
            if (!isDrawing) {
                startPoint = event->pos();
                isDrawing = true;
            } else {
                endPoint = event->pos();
                isDrawing = false;
                update();
            }
        }
    }

    void handlemouseMoveEvent(QMouseEvent *event){
        qDebug() << "mouseMovePainter";
        if (isDrawing) {
            endPoint = event->pos();
            update();
        }
    }

    void paintEvent(QPaintEvent *event) override {
        QWidget::paintEvent(event);
        if (isDrawing || !endPoint.isNull()) {
            QPainter painter(this);
            painter.setPen(QPen(Qt::red, 2));
            painter.drawLine(startPoint, endPoint);
        }
    }
private:
    QPoint startPoint;
    QPoint endPoint;
    bool isDrawing = false;
    LineType curLineType;
};

class TVideoWdg : public QVideoWidget
{
public:
    TVideoWdg(QWidget *parent = nullptr);
protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            overlay->handlePressEvent(event);
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        overlay->handlemouseMoveEvent(event);
    }

    void paintEvent(QPaintEvent *event) override {
        QVideoWidget::paintEvent(event);
    }

    bool eventFilter(QObject* obj, QEvent* event) override {
        if (obj == this) {
            switch (event->type()) {
            case QEvent::Move:
            case QEvent::Resize:
            case QEvent::Show:
            case QEvent::Hide:
            case QEvent::WindowActivate:
                updateOverlayPosition();
                break;
            default:
                break;
            }
        }
        return QVideoWidget::eventFilter(obj, event);
    }

    void updateOverlayPosition() {
        if (!overlay) return;

        if (isVisible()) {
            QPoint globalPos = mapToGlobal(QPoint(0, 0));
            overlay->setGeometry(QRect(globalPos, size()));
            overlay->show();
            overlay->raise();
        } else {
            overlay->hide();
        }
    }
private:
    TLinePainter *overlay;
};

#endif // TVIDEOWDG_H
