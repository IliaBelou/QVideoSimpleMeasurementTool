#include "tvideowdg.h"

TVideoWdg::TVideoWdg(QWidget *parent)
    : QVideoWidget(parent)
{
    overlay = new TLinePainter(nullptr);
    overlay->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    overlay->setAttribute(Qt::WA_TranslucentBackground);

    installEventFilter(this);
    updateOverlayPosition();
}

TLinePainter::TLinePainter(QWidget *parent)
    : QWidget(parent)
{

}
