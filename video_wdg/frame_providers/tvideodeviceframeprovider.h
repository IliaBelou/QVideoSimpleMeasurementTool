/** @file
 *
 * TVideoWdg - SimpleMeasurementByVideoTool
 * By Ilia.belou <ilia.belou@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TVIDEODEVICEFRAMEPROVIDER_H
#define TVIDEODEVICEFRAMEPROVIDER_H

#include <QObject>
#include <QCamera>
#include <QCameraDevice>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QMediaDevices>
#include <QVideoFrame>
#include <QThread>

#include "iframeprovider.h"

/*!
 * \class TVideoDeviceFrameProvider
 * \brief Frame provider for USB video devices using Qt's camera framework.
 *
 * The `TVideoDeviceFrameProvider` class implements the `IFrameProvider` interface to capture video frames from USB
 * cameras using Qt's `QCamera` and `QMediaCaptureSession`. It supports selecting devices, configuring video formats,
 * and processing frames asynchronously via a `QVideoSink`. Frames are provided as `QImage` objects with thread-safe access.
 */
class TVideoDeviceFrameProvider : public IFrameProvider
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a TVideoDeviceFrameProvider instance.
     * \param parent The parent QObject (default is nullptr).
     */
    explicit TVideoDeviceFrameProvider(QObject* parent = nullptr);

    /*!
     * \brief Destructor.
     *
     * Stops the camera and cleans up resources.
     */
    ~TVideoDeviceFrameProvider();

    /*!
     * \brief Retrieves descriptions of available camera devices.
     * \return A list of camera device descriptions.
     */
    QList<std::string> getDeviceDesc() override;

    /*!
     * \brief Sets the active camera device by its description.
     * \param desc The description of the camera device to activate.
     */
    void setDeviceByDesc(std::string desc) override;

    /*!
     * \brief Retrieves available video formats for the current camera.
     * \return A list of format descriptions (width,height).
     */
    QList<std::string> getCurrentDeviceAvaliableFormats() override;

    /*!
     * \brief Sets the video format for the current camera by index.
     * \param idx The index of the format in the list of available formats.
     */
    void setCurrentDeviceFormatByIdx(int idx) override;

    /*!
     * \brief Sets the URL for the provider.
     * \param url The URL (currently unused).
     *
     * This implementation does not use URLs, as it is designed for physical camera devices.
     */
    void setUrl(std::string url) override;

protected:
    void run() override;

private slots:
    void updateFrame(const QVideoFrame &frame);

private:
    std::unique_ptr<QCamera> camera_;                   ///< Qt camera object.
    std::unique_ptr<QMediaCaptureSession> captureSession_; ///< Media capture session for the camera.
    std::unique_ptr<QVideoSink> videoSink_;             ///< Video sink for receiving frames.
    QMediaDevices mediaDevices_;                        ///< Media devices manager.
    QList<QCameraDevice> cameras_;                      ///< List of available camera devices.
    QList<QCameraFormat> formats_;                      ///< List of available video formats for the current camera.
    QString currentVideoSourceDesc_;                    ///< Description of the current video source.
    QString currentVideoFormatDesc_;                    ///< Description of the current video format.
};

#endif // TVIDEODEVICEFRAMEPROVIDER_H
