#include "tvideodeviceframeprovider.h"
#include <QDebug>

TVideoDeviceFrameProvider::TVideoDeviceFrameProvider(QObject* parent)
    : IFrameProvider(parent), videoSink_(new QVideoSink(this))
{
}

TVideoDeviceFrameProvider::~TVideoDeviceFrameProvider()
{
    if (camera_) {
        camera_->stop();
    }
    delete camera_;
    delete captureSession_;
    delete videoSink_;
}

QList<std::string> TVideoDeviceFrameProvider::getDeviceDesc()
{
    QList<std::string> list;
    for (const QCameraDevice &cam : cameras_) {
        list.append(cam.description().toStdString());
    }
    return list;
}

void TVideoDeviceFrameProvider::setDeviceByDesc(std::string desc)
{
    qDebug() << "setDeviceByDesc thread:" << QThread::currentThread();
    for (const QCameraDevice &cam : cameras_) {
        if (desc == cam.description().toStdString()) {
            camera_->stop();
            camera_->setCameraDevice(cam);
            formats_ = cam.videoFormats();
            if (!formats_.isEmpty()) {
                camera_->setCameraFormat(formats_.first());
            }
            captureSession_->setCamera(camera_);
            captureSession_->setVideoSink(videoSink_);
            camera_->start();
            break;
        }
    }
}

QList<std::string> TVideoDeviceFrameProvider::getCurrentDeviceAvaliableFormats()
{
    QList<std::string> list;
    for (const QCameraFormat &fmt : formats_) {
        list.append(QString("%1,%2")
                        .arg(fmt.resolution().width())
                        .arg(fmt.resolution().height()).toStdString());
    }
    return list;
}

void TVideoDeviceFrameProvider::setCurrentDeviceFormatByIdx(int idx)
{
    if (idx < 0 || idx >= formats_.size()) return;
    QMetaObject::invokeMethod(this, [this, idx]() {
        camera_->setCameraFormat(formats_[idx]);
        camera_->start();
    }, Qt::QueuedConnection);
}

void TVideoDeviceFrameProvider::setUrl(std::string)
{
}

void TVideoDeviceFrameProvider::run()
{
    qDebug() << "run thread:" << QThread::currentThread();
    cameras_ = mediaDevices_.videoInputs();
    if (!cameras_.isEmpty()) {
        camera_ = new QCamera(cameras_.first(), this);
        captureSession_ = new QMediaCaptureSession(this);
        captureSession_->setCamera(camera_);
        captureSession_->setVideoSink(videoSink_);

        formats_ = cameras_.first().videoFormats();
        if (!formats_.isEmpty()) {
            camera_->setCameraFormat(formats_.first());
        }

        bool connected = connect(videoSink_, &QVideoSink::videoFrameChanged,
                                 this, &TVideoDeviceFrameProvider::updateFrame,
                                 Qt::QueuedConnection);
        qDebug() << "Connection established:" << connected;

        camera_->start();
        qDebug() << "Camera state:" << camera_->isActive();
        qDebug() << "Camera error:" << camera_->error();
        qDebug() << "Active format:" << camera_->cameraFormat().resolution();
        qDebug() << "Supported formats:" << formats_.size();
    }
    isRunning_ = true;
    ready_();
}

void TVideoDeviceFrameProvider::updateFrame(const QVideoFrame &frame)
{
    std::lock_guard<std::mutex> lock(framemtx_);
    frame_ = frame.toImage();
    frameReady_ = true;
}
