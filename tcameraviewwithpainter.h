#ifndef TCAMERAVIEWWITHPAINTER_H
#define TCAMERAVIEWWITHPAINTER_H

#include <QObject>
#include <QGraphicsView>
#include <QMouseEvent>

class TCameraViewWithPainter : public QGraphicsView
{
    Q_OBJECT
public:
    TCameraViewWithPainter(QWidget *parent = nullptr) : QGraphicsView(parent) {};
signals:
    void mousePressed(QMouseEvent *event);
    void mouseMoved(QMouseEvent *event);
    void mouseReleased(QMouseEvent *event);
    void resized();
protected:
    void mousePressEvent(QMouseEvent *event) override { emit mousePressed(event); }
    void mouseMoveEvent(QMouseEvent *event) override { emit mouseMoved(event); }
    void mouseReleaseEvent(QMouseEvent *event) override { emit mouseReleased(event); }
    void resizeEvent(QResizeEvent *event) override {
        QGraphicsView::resizeEvent(event);
        emit resized();
    }
private:

};

#endif // TCAMERAVIEWWITHPAINTER_H
