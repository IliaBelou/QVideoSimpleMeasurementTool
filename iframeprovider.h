#ifndef IFRAMEPROVIDER_H
#define IFRAMEPROVIDER_H

#include <QImage>
#include <QList>
#include <mutex>
#include <thread>
#include <QObject>

class IFrameProvider : public QObject
{
    Q_OBJECT
public:
    explicit IFrameProvider() = default;
    virtual QList<std::string> getDeviceDesc() = 0;
    virtual void setDeviceByDesc(std::string desc) = 0;
    virtual QList<std::string> getCurrentDeviceAvaliableFormats() = 0;
    virtual void setCurrentDeviceFormatByIdx(int idx) = 0;
    virtual void setUrl(std::string) = 0;
    const std::atomic<bool>& isReady() {
        return frameReady_;
    }
    const std::atomic<bool>& isRunning() {
        return isRunning_;
    }
    const QImage& getFrame() {
        std::lock_guard<std::mutex> lock(framemtx_);
        frameReady_ = false;
        return frame_;
    }
    virtual ~IFrameProvider() = default;
    void start(std::function<void()> ready) {
        ready_ = ready;
        if (isRunning_) return;
        isRunning_ = true;
        workerThread_ = std::thread(&IFrameProvider::run, this);
    }
    void stop() {
        isRunning_ = false;
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    };
protected:
    virtual void run() = 0;
    std::thread workerThread_;
    std::atomic<bool> isRunning_{false};
    std::atomic<bool> frameReady_{false};
    std::mutex framemtx_;
    QImage frame_;
    std::function<void()> ready_;
};


#endif // IFRAMEPROVIDER_H
