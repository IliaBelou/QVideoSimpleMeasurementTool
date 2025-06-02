#ifndef IFRAMEMIDDLEWARE_H
#define IFRAMEMIDDLEWARE_H

#include <QImage>

class IFrameMiddleware
{
public:
    IFrameMiddleware() = default;
    virtual void processFrame(QImage* img) = 0;
};

#endif // IFRAMEMIDDLEWARE_H
