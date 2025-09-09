/** @file
 *
 * TVideoWdg - SimpleMeasurementByVideoTool
 * By Ilia.belou <ilia.belou@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef IFRAMEMIDDLEWARE_H
#define IFRAMEMIDDLEWARE_H

#include <QImage>

/*!
 * \class IFrameMiddleware
 * \brief Abstract interface for processing video frames.
 *
 * The `IFrameMiddleware` class is a pure virtual interface that defines the contract for classes that process video frames
 * represented as `QImage` objects. Derived classes must implement the `processFrame` method to apply specific image
 * processing operations, such as edge detection or filtering. This interface is designed to be used in a video processing
 * pipeline, allowing modular and extensible frame manipulation.
 */
class IFrameMiddleware
{
public:
    /*!
     * \brief Default constructor.
     */
    IFrameMiddleware() = default;

    /*!
     * \brief Processes a video frame.
     * \param img Pointer to the QImage to be processed.
     *
     * Must be implemented by derived classes to perform specific image processing operations on the provided frame.
     * The image may be modified in-place.
     */
    virtual void processFrame(QImage* img) = 0;

    /*!
     * \brief Virtual destructor.
     *
     * Ensures proper cleanup in derived classes.
     */
    virtual ~IFrameMiddleware() = default;
};

#endif // IFRAMEMIDDLEWARE_H
