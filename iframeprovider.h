#ifndef IFRAMEPROVIDER_H
#define IFRAMEPROVIDER_H

#include <QImage>
#include <QList>
#include <mutex>
#include <QObject>
#include <QThread>

class IFrameProvider : public QObject
{
    Q_OBJECT
public:
    explicit IFrameProvider(QObject* parent = nullptr) : QObject(parent) {}
    virtual QList<std::string> getDeviceDesc() = 0;
    virtual void setDeviceByDesc(std::string desc) = 0;
    virtual QList<std::string> getCurrentDeviceAvaliableFormats() = 0;
    virtual void setCurrentDeviceFormatByIdx(int idx) = 0;
    virtual void setUrl(std::string url) = 0;

    const std::atomic<bool>& isReady() const {
        return frameReady_;
    }
    const std::atomic<bool>& isRunning() const {
        return isRunning_;
    }
    const QImage& getFrame() {
        std::lock_guard<std::mutex> lock(framemtx_);
        frameReady_ = false;
        return frame_;
    }

    virtual ~IFrameProvider() {
        stop();
    }

    void start(std::function<void()> ready) {
        if (isRunning_) return;
        ready_ = ready;
        isRunning_ = true;
        workerThread_ = new QThread(this);
        this->moveToThread(workerThread_);
        connect(workerThread_, &QThread::started, this, &IFrameProvider::run);
        workerThread_->start();
    }

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
    virtual void run() = 0;

    QThread* workerThread_ = nullptr;
    std::atomic<bool> isRunning_{false};
    std::atomic<bool> frameReady_{false};
    std::mutex framemtx_;
    QImage frame_;
    std::function<void()> ready_;
};

#endif // IFRAMEPROVIDER_H
