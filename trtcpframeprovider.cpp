#include "trtcpframeprovider.h"

TRTCPFrameProvider::TRTCPFrameProvider(QObject *parent)
    : IFrameProvider{parent}
{}

QList<std::string> TRTCPFrameProvider::getDeviceDesc()
{
    return QList<std::string> {url_};
}

void TRTCPFrameProvider::setDeviceByDesc(std::string desc)
{

}

QList<std::string> TRTCPFrameProvider::getCurrentDeviceAvaliableFormats()
{
    double width = rtspCapture_.get()->get(cv::CAP_PROP_FRAME_WIDTH);
    double height = rtspCapture_.get()->get(cv::CAP_PROP_FRAME_HEIGHT);
    return QList<std::string>{QString("%1,%2").arg(width).arg(height).toStdString()};
}

void TRTCPFrameProvider::setCurrentDeviceFormatByIdx(int idx)
{

}

void TRTCPFrameProvider::setUrl(std::string url)
{
    url_ = url;
}

void TRTCPFrameProvider::run()
{
    startRtspCapture();
}

void TRTCPFrameProvider::startRtspCapture()
{
    rtspCapture_.reset(new cv::VideoCapture(url_, cv::CAP_FFMPEG));
    if (!rtspCapture_->isOpened()) {
        qDebug() << "Failed to open RTSP stream:" << url_;
        return;
    }

    rtspTimer_.reset(new QTimer(this));
    connect(rtspTimer_.get(), &QTimer::timeout, this,&TRTCPFrameProvider::processRtspFrame,Qt::UniqueConnection);
    rtspTimer_->start(33);
    ready_();
}

void TRTCPFrameProvider::stopRtspCapture()
{
    if (rtspTimer_.get() != nullptr) {
        rtspTimer_->stop();
    }
}

void TRTCPFrameProvider::processRtspFrame()
{
    if (!rtspCapture_ || !rtspCapture_->isOpened()) {
        qDebug() << "RTSP capture not opened";
        //stopRtspCapture();
        return;
    }

    cv::Mat frame;
    if (!rtspCapture_->read(frame)) {
        qDebug() << "Failed to read RTSP frame";
        return;
    }

    if (frame.empty()) {
        qDebug() << "Empty RTSP frame";
        return;
    }

    cv::Mat processed;
    if (frame.type() == CV_8UC3) {
        cv::cvtColor(frame, processed, cv::COLOR_BGR2BGRA);
    } else if (frame.type() == CV_8UC4) {
        processed = frame.clone();
    } else {
        qDebug() << "Unsupported RTSP frame type:" << frame.type();
        return;
    }

    QImage outputImage((const uchar*)processed.data, processed.cols, processed.rows, processed.step, QImage::Format_RGB32);
    outputImage = outputImage.copy();
    {
        std::lock_guard<std::mutex> lock(framemtx_);
        frame_ = outputImage;
        frameReady_ = true;
    }
}
