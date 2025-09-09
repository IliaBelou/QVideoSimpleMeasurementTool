/** @file
 *
 * TVideoWdg - SimpleMeasurementByVideoTool
 * By Ilia.belou <ilia.belou@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TEDGEDETECTOR_H
#define TEDGEDETECTOR_H

#include <QDebug>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "iframemiddleware.h"
#include "video_wdg/cv_to_qt_image/cvmatandqimage.h"

/*!
 * \class TEdgeDetector
 * \brief Frame middleware for applying Canny edge detection to video frames.
 *
 * The `TEdgeDetector` class implements the `IFrameMiddleware` interface to perform edge detection on video frames using
 * OpenCV's Canny algorithm. It converts input `QImage` frames to OpenCV `cv::Mat`, applies grayscale conversion, Gaussian
 * blur, and Canny edge detection, then converts the result back to a `QImage`. The class supports configurable thresholds
 * for the Canny algorithm and ensures input images are in a compatible format (`Format_RGB32` or `Format_ARGB32`).
 */
class TEdgeDetector : public IFrameMiddleware
{
public:
    /*!
     * \brief Constructs a TEdgeDetector instance.
     * \param thr1 The first threshold for the Canny edge detection (default is 100.0).
     * \param thr2 The second threshold for the Canny edge detection (default is 200.0).
     *
     * Initializes the edge detector with the specified thresholds for the Canny algorithm.
     */
    TEdgeDetector(double thr1 = 100.0,double thr2 = 200.0);

    /*!
     * \brief Processes a video frame by applying Canny edge detection.
     * \param img Pointer to the QImage to be processed.
     *
     * Converts the input image to `Format_RGB32` if necessary, applies grayscale conversion, Gaussian blur,
     * and Canny edge detection, then updates the image with the detected edges.
     */
    void processFrame(QImage* img) override;
private:
    double thr1_ = 100.0; ///< First threshold for Canny edge detection.
    double thr2_ = 200.0; ///< Second threshold for Canny edge detection.
};

#endif // TEDGEDETECTOR_H
