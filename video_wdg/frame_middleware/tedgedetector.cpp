#include "tedgedetector.h"

TEdgeDetector::TEdgeDetector(double thr1, double thr2) :
    thr1_(thr1),
    thr2_(thr2)
{}

void TEdgeDetector::processFrame(QImage *img)
{
    if (img->isNull()) {
        qDebug() << "Null QImage";
        return;
    }

    qDebug() << "QImage format:" << img->format() << "size:" << img->size();

    // Convert and copy QImage to ensure stable memory
    if (img->format() != QImage::Format_RGB32 && img->format() != QImage::Format_ARGB32) {
        qDebug() << "Converting QImage to Format_RGB32";
        *img = img->convertToFormat(QImage::Format_RGB32).copy(); // Deep copy
        if (img->isNull()) {
            qDebug() << "Failed to convert QImage to Format_RGB32";
            return;
        }
    }

    cv::Mat mat = QtOcv::image2Mat(*img);
    if (mat.empty()) {
        qDebug() << "Empty cv::Mat";
        return;
    }
    qDebug() << "cv::Mat size:" << mat.cols << "x" << mat.rows << "type:" << mat.type();

    cv::Mat processed = mat.clone();
    if (processed.empty()) {
        qDebug() << "Empty processed cv::Mat";
        return;
    }


    cv::Mat gray;
    qDebug() << "Before cvtColor: processed size:" << processed.cols << "x" << processed.rows << "type:" << processed.type();
    try {
        cv::cvtColor(processed, gray, cv::COLOR_BGR2GRAY);
    } catch (const cv::Exception& e) {
        qDebug() << "cv::cvtColor failed:" << e.what();
        return;
    }
    if (gray.empty()) {
        qDebug() << "Empty gray cv::Mat after cvtColor";
        return;
    }
    qDebug() << "Gray size:" << gray.cols << "x" << gray.rows << "type:" << gray.type();

    cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);
    cv::Mat edges;
    cv::Canny(gray, edges, thr1_, thr2_);
    cv::cvtColor(edges, processed, cv::COLOR_GRAY2BGRA);

    *img = QtOcv::mat2Image(edges);
}
