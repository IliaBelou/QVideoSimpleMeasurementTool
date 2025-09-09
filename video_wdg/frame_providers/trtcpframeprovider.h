/** @file
 *
 * TVideoWdg - SimpleMeasurementByVideoTool
 * By Ilia.belou <ilia.belou@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TRTCPFRAMEPROVIDER_H
#define TRTCPFRAMEPROVIDER_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "iframeprovider.h"

/*!
 * \class TRTCPFrameProvider
 * \brief Frame provider for RTSP video streams using OpenCV.
 *
 * The `TRTCPFrameProvider` class implements the `IFrameProvider` interface to capture video frames from RTSP streams
 * using OpenCV's `VideoCapture`. It supports configuring the stream URL and periodically processes frames using a timer.
 * The class converts captured frames to a compatible format (`QImage::Format_RGB32`) and provides thread-safe access to them.
 */
class TRTCPFrameProvider : public IFrameProvider
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a TRTCPFrameProvider instance.
     * \param parent The parent QObject (default is nullptr).
     */
    explicit TRTCPFrameProvider(QObject *parent = nullptr);

    /*!
     * \brief Destructor.
     *
     * Inherits the base class destructor, which stops the provider.
     */
    ~TRTCPFrameProvider() = default;

    /*!
     * \brief Retrieves the description of the RTSP source.
     * \return A list containing the current RTSP URL.
     */
    QList<std::string> getDeviceDesc() override;

    /*!
     * \brief Sets the active device by description.
     * \param desc The device description (currently unused).
     *
     * This implementation does not use the description, as RTSP sources are defined by URL.
     */
    void setDeviceByDesc(std::string desc) override;

    /*!
     * \brief Retrieves the available formats for the current RTSP stream.
     * \return A list containing the resolution of the stream (width,height).
     */
    QList<std::string> getCurrentDeviceAvaliableFormats() override;

    /*!
     * \brief Sets the video format by index.
     * \param idx The format index (currently unused).
     *
     * This implementation does not support format changes for RTSP streams.
     */
    void setCurrentDeviceFormatByIdx(int idx) override;

    /*!
     * \brief Sets the RTSP stream URL.
     * \param url The RTSP URL to capture from.
     */
    void setUrl(std::string url) override;

protected:
    void run() override;

private slots:
    /*!
     * \brief Starts the RTSP capture and timer.
     */
    void startRtspCapture();

    /*!
     * \brief Stops the RTSP capture and timer.
     */
    void stopRtspCapture();

    /*!
     * \brief Processes a single RTSP frame.
     *
     * Captures a frame from the RTSP stream, converts it to the required format, and updates the frame buffer.
     */
    void processRtspFrame();
private:
    std::unique_ptr<cv::VideoCapture> rtspCapture_= nullptr;   ///< OpenCV video capture for RTSP stream.
    std::unique_ptr<QTimer> rtspTimer_ = nullptr;              ///< Timer for periodic frame processing.
    std::string url_{};                                        ///< URL of the RTSP stream.

};

#endif // TRTCPFRAMEPROVIDER_H
