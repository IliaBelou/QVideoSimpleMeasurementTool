#ifndef TEDGEDETECTOR_H
#define TEDGEDETECTOR_H

#include <QDebug>
#include "iframemiddleware.h"
#include "cvmatandqimage.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

class TEdgeDetector : public IFrameMiddleware
{
public:
    TEdgeDetector(double thr1 = 100.0,double thr2 = 200.0);
    void processFrame(QImage* img) override;
private:
    double thr1_ = 100.0;
    double thr2_ = 200.0;
};

#endif // TEDGEDETECTOR_H
