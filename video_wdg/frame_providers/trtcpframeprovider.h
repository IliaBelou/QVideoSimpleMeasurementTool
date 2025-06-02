#ifndef TRTCPFRAMEPROVIDER_H
#define TRTCPFRAMEPROVIDER_H

#include <QObject>
#include <QTimer>
#include <QDebug>

#include "iframeprovider.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

class TRTCPFrameProvider : public IFrameProvider
{
    Q_OBJECT
public:
    explicit TRTCPFrameProvider(QObject *parent = nullptr);
    ~TRTCPFrameProvider() = default;
    QList<std::string> getDeviceDesc() override;
    void setDeviceByDesc(std::string desc) override;
    QList<std::string> getCurrentDeviceAvaliableFormats() override;
    void setCurrentDeviceFormatByIdx(int idx) override;
    void setUrl(std::string url) override;

protected:
    void run() override;

private slots:

private:
    std::unique_ptr<cv::VideoCapture> rtspCapture_= nullptr;
    std::unique_ptr<QTimer> rtspTimer_ = nullptr;
    std::string url_{};
    void startRtspCapture();
    void stopRtspCapture();
    void processRtspFrame();
};

#endif // TRTCPFRAMEPROVIDER_H
