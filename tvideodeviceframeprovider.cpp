#include "tvideodeviceframeprovider.h"
#include <qapplication.h>

TVideoDeviceFrameProvider::TVideoDeviceFrameProvider() :
    videoSink_(new QVideoSink())
{

}

QList<std::string> TVideoDeviceFrameProvider::getDeviceDesc()
{
    if (!isRunning_) return QList<std::string>{};
    QList<std::string> list;
    for (const QCameraDevice &cam : cameras_) {
        list.append(cam.description().toStdString());
    }
    return list;
}

void TVideoDeviceFrameProvider::setDeviceByDesc(std::string desc)
{
    if (!isRunning_) return;
    QMetaObject::invokeMethod(this, [this, desc]() {
    qDebug() << "setDeviceByDesc thread:" << QThread::currentThread();
        for (const QCameraDevice &cam : cameras_) {
            if (desc == cam.description()) {
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
    }, Qt::QueuedConnection);
}

QList<std::string> TVideoDeviceFrameProvider::getCurrentDeviceAvaliableFormats()
{
    if (!isRunning_) return QList<std::string>{};
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
    if (!isRunning_) return;
    if (idx < 0 || idx > formats_.size()) return;
    camera_->setCameraFormat(formats_[idx]);
}

void TVideoDeviceFrameProvider::setUrl(std::string)
{

}

void TVideoDeviceFrameProvider::run()
{
    // Все объекты создаём в этом же потоке
    cameras_ = mediaDevices_.videoInputs();
    if (!cameras_.isEmpty()) {
        qDebug() << "run thread:" << QThread::currentThread();
        camera_ = new QCamera(cameras_.first());
        captureSession_ = new QMediaCaptureSession();
        captureSession_->setCamera(camera_);
        captureSession_->setVideoSink(videoSink_);

        formats_ = cameras_.first().videoFormats();
        if (!formats_.isEmpty()) {
            camera_->setCameraFormat(formats_.first());
        }

        bool connected = connect(videoSink_, &QVideoSink::videoFrameChanged,
                                 this, &TVideoDeviceFrameProvider::updateFrame,
                                 Qt::UniqueConnection);
        qDebug() << "Connection established:" << connected;

        camera_->start();
        qDebug() << "Camera state:" << camera_->isActive();
        qDebug() << "Camera error:" << camera_->error();
        qDebug() << "Active format:" << camera_->cameraFormat().resolution();
        qDebug() << "Supported formats:" << formats_.size();
    }
    isRunning_ = true;
    ready_();

    // Основной цикл обработки
    while (isRunning_) {
        QCoreApplication::processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void TVideoDeviceFrameProvider::updateFrame(const QVideoFrame &frame)
{
    qDebug() << "updateFrame thread:" << QThread::currentThread();
    std::lock_guard<std::mutex> lock(framemtx_);
    frame_ = frame.toImage();
    frameReady_ = true;
}
