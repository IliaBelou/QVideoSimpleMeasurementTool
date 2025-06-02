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

class TVideoDeviceFrameProvider : public IFrameProvider
{
    Q_OBJECT
public:
    explicit TVideoDeviceFrameProvider(QObject* parent = nullptr);
    ~TVideoDeviceFrameProvider();
    QList<std::string> getDeviceDesc() override;
    void setDeviceByDesc(std::string desc) override;
    QList<std::string> getCurrentDeviceAvaliableFormats() override;
    void setCurrentDeviceFormatByIdx(int idx) override;
    void setUrl(std::string url) override;

protected:
    void run() override;

private slots:
    void updateFrame(const QVideoFrame &frame);

private:
    QCamera* camera_ = nullptr;
    QMediaCaptureSession* captureSession_ = nullptr;
    QVideoSink* videoSink_ = nullptr;
    QMediaDevices mediaDevices_;
    QList<QCameraDevice> cameras_;
    QList<QCameraFormat> formats_;
    QString curVideoSourceDesc;
    QString curVideoFormatDesc;
};

#endif // TVIDEODEVICEFRAMEPROVIDER_H
