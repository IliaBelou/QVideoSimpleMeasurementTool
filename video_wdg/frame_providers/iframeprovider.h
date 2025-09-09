/** @file
 *
 * TVideoWdg - SimpleMeasurementByVideoTool
 * By Ilia.belou <ilia.belou@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef IFRAMEPROVIDER_H
#define IFRAMEPROVIDER_H

#include <QImage>
#include <QList>
#include <mutex>
#include <QObject>
#include <QThread>

/*!
 * \class IFrameProvider
 * \brief Abstract interface for providing video frames from various sources.
 *
 * The `IFrameProvider` class is a pure virtual interface that defines the contract for classes that provide video frames,
 * such as from USB cameras or RTSP streams. It manages frame acquisition in a separate thread, ensuring thread-safe access
 * to frames via a mutex-protected `QImage`. Derived classes must implement methods for device management, format selection,
 * and frame capture. The class supports starting and stopping frame acquisition, with readiness notifications via a callback.
 */
class IFrameProvider : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs an IFrameProvider instance.
     * \param parent The parent QObject (default is nullptr).
     */
    explicit IFrameProvider(QObject* parent = nullptr) : QObject(parent) {}

    /*!
     * \brief Destructor.
     *
     * Stops the frame acquisition thread and cleans up resources.
     */
    virtual ~IFrameProvider() {
        stop();
    }

    /*!
     * \brief Retrieves descriptions of available devices.
     * \return A list of device descriptions as strings.
     *
     * Must be implemented by derived classes to return identifiers for available video sources.
     */
    virtual QList<std::string> getDeviceDesc() = 0;

    /*!
     * \brief Sets the active device by its description.
     * \param desc The description of the device to activate.
     *
     * Must be implemented by derived classes to select a specific video source.
     */
    virtual void setDeviceByDesc(std::string desc) = 0;

    /*!
     * \brief Retrieves available video formats for the current device.
     * \return A list of format descriptions (e.g., resolution) as strings.
     *
     * Must be implemented by derived classes to return supported formats for the active device.
     */
    virtual QList<std::string> getCurrentDeviceAvaliableFormats() = 0;

    /*!
     * \brief Sets the video format for the current device by index.
     * \param idx The index of the format in the list of available formats.
     *
     * Must be implemented by derived classes to configure the video format.
     */
    virtual void setCurrentDeviceFormatByIdx(int idx) = 0;

    /*!
     * \brief Sets the URL for streaming sources.
     * \param url The URL of the video source (e.g., RTSP stream).
     *
     * Must be implemented by derived classes to configure streaming sources, if applicable.
     */
    virtual void setUrl(std::string url) = 0;

    /*!
     * \brief Checks if a new frame is ready.
     * \return Reference to the atomic boolean indicating frame readiness.
     */
    const std::atomic<bool>& isReady() const {
        return frameReady_;
    }

    /*!
     * \brief Checks if the provider is running.
     * \return Reference to the atomic boolean indicating running state.
     */
    const std::atomic<bool>& isRunning() const {
        return isRunning_;
    }

    /*!
     * \brief Retrieves the latest video frame.
     * \return Reference to the QImage containing the frame.
     *
     * Provides thread-safe access to the current frame, resetting the readiness flag.
     */
    const QImage& getFrame() {
        std::lock_guard<std::mutex> lock(framemtx_);
        frameReady_ = false;
        return frame_;
    }

    /*!
     * \brief Starts frame acquisition.
     * \param ready Callback function to be called when the provider is ready.
     *
     * Launches frame acquisition in a separate thread and invokes the ready callback when initialized.
     */
    void start(std::function<void()> ready) {
        if (isRunning_) return;
        ready_ = ready;
        isRunning_ = true;
        workerThread_ = new QThread(this);
        this->moveToThread(workerThread_);
        connect(workerThread_, &QThread::started, this, &IFrameProvider::run);
        workerThread_->start();
    }

    /*!
     * \brief Stops frame acquisition.
     *
     * Terminates the worker thread and cleans up resources.
     */
    void stop() {
        if (isRunning_) {
            isRunning_ = false;
            if (workerThread_) {
                workerThread_->quit();
                workerThread_->wait();
                delete workerThread_;
                workerThread_ = nullptr;
            }
        }
    }

protected:
    /*!
     * \brief Executes the frame acquisition loop.
     *
     * Must be implemented by derived classes to define the frame capture process.
     */
    virtual void run() = 0;

    QThread* workerThread_ = nullptr;                ///< Worker thread for frame acquisition.
    std::atomic<bool> isRunning_{false};            ///< Flag indicating if the provider is running.
    std::atomic<bool> frameReady_{false};           ///< Flag indicating if a new frame is ready.
    std::mutex framemtx_;                           ///< Mutex for thread-safe frame access.
    QImage frame_;                                  ///< Current video frame.
    std::function<void()> ready_;                   ///< Callback invoked when the provider is ready.
};

#endif // IFRAMEPROVIDER_H
