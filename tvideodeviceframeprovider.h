#ifndef TVIDEODEVICEFRAMEPROVIDER_H
#define TVIDEODEVICEFRAMEPROVIDER_H

#include <QObject>
#include <QCamera>
#include <QCameraDevice>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QMediaDevices>
#include <QVideoFrame>
#include <QApplication>
#include <QThread>

#include "iframeprovider.h"

class TVideoDeviceFrameProvider : public IFrameProvider
{
    Q_OBJECT
public:
    TVideoDeviceFrameProvider();
    void extracted(QList<std::string> &list);
    QList<std::string> getDeviceDesc() override;
    void setDeviceByDesc(std::string desc) override;
    QList<std::string> getCurrentDeviceAvaliableFormats() override;
    void setCurrentDeviceFormatByIdx(int idx) override;
    void setUrl(std::string) override;
    void run() override;
private:
    QCamera* camera_;
    QMediaCaptureSession *captureSession_;
    QVideoSink *videoSink_;
    QMediaDevices mediaDevices_;
    QList<QCameraDevice> cameras_;
    QList<QCameraFormat> formats_;
    QString curVideoSourceDesc;
    QString curVideoFormatDesc;
private slots:
    void updateFrame(const QVideoFrame &frame);
};

#endif // TVIDEODEVICEFRAMEPROVIDER_H
