#include "tvideowdg.h"
#include "tedgedetector.h"
#include "trtcpframeprovider.h"
#include "tvideodeviceframeprovider.h"

TVideoWdg::TVideoWdg(QWidget *parent) :
    QGraphicsView(parent),
    scene_(new QGraphicsScene(this)),
    painter_(new TSurfacePainter(scene_)),
    currentFrame_(new QGraphicsPixmapItem)
{
    // Painter
    connect(this,&TVideoWdg::mouseMoved,painter_,&TSurfacePainter::handleMouseMoved);
    connect(this,&TVideoWdg::mousePressed,painter_,&TSurfacePainter::handleMousePressed);
    connect(this,&TVideoWdg::mouseReleased,painter_,&TSurfacePainter::handleMouseReleased);

    // Scene
    this->setScene(scene_);
    scene_->addItem(currentFrame_);
    // FrameProviders
    usbDevs_ = new TVideoDeviceFrameProvider;
    fproviders_.append(usbDevs_);
    auto usbDevsReady = [this]() {
        videosrcDesc_.append(usbDevs_->getDeviceDesc());
        videofmtDesc_.clear();
        videofmtDesc_.append(usbDevs_->getCurrentDeviceAvaliableFormats());
        emit videoSourcesChanged(videosrcDesc_);
        emit videoFormatsChanged(videofmtDesc_);
    };
    usbDevs_->start(usbDevsReady);

    // Frame update
    updateFrame_ = new QTimer;
    connect(updateFrame_,&QTimer::timeout,[this]{
        updateFrame();
    });
    updateFrame_->start(FRAME_UPDATE_PERIOD);
}

TVideoWdg::~TVideoWdg()
{
    delete painter_;
    delete scene_;
}

TSurfacePainter *TVideoWdg::getPainter()
{
    return painter_;
}

void TVideoWdg::updateFrame()
{
    if (fproviders_.at(currentActiveVideoProviderIdx_)->isReady()) {
        currentFrameImg_ = fproviders_.at(currentActiveVideoProviderIdx_)->getFrame();
        if (!fmiddlewares_.empty()) {
            for (const auto& mw : fmiddlewares_) {
                mw->processFrame(&currentFrameImg_);
            }
        }
        currentFrame_->setPixmap(QPixmap::fromImage(currentFrameImg_));
        updateVideoSize(currentFrameImg_);
        scene_->update();
    }
}

void TVideoWdg::changeVideoSrc(const QString &src)
{
    for (int i =0;i < fproviders_.size();i++) {
        QList<std::string> avaliableDeviceDesc = fproviders_.at(i)->getDeviceDesc();
        for (const std::string& deviceDesc : avaliableDeviceDesc) {
            if (src.toStdString() == deviceDesc) {
                currentActiveVideoProviderIdx_ = i;
                fproviders_.at(i)->setDeviceByDesc(src.toStdString());
                QList<std::string> fmts = fproviders_.at(i)->getCurrentDeviceAvaliableFormats();
                emit videoFormatsChanged(fmts);
            }
        }
    }
}

void TVideoWdg::changeVideofmt(int idx)
{
    fproviders_.at(currentActiveVideoProviderIdx_)->setCurrentDeviceFormatByIdx(idx);
}

void TVideoWdg::incZoom()
{
    zoomFactor_ += ZOOM_FACTOR;
    if (zoomFactor_ > 1.0) {
        zoomFactor_ = 1.0;
    }
    resetTransform();
    scale(zoomFactor_, zoomFactor_);
    fitInView(currentFrame_, Qt::KeepAspectRatio);
    scene_->update();
}

void TVideoWdg::decZoom()
{
    zoomFactor_ -= ZOOM_FACTOR;
    if (zoomFactor_ < 0.05) {
        zoomFactor_ = 0.05;
    }
    resetTransform();
    scale(zoomFactor_, zoomFactor_);
    fitInView(currentFrame_, Qt::KeepAspectRatio);
    scene_->update();
}

void TVideoWdg::fit()
{
    scene_->setSceneRect(0, 0, currentFrameImg_.width(), currentFrameImg_.height());
    zoomFactor_ = 1.0;
    resetTransform();
    fitInView(currentFrame_, Qt::KeepAspectRatio);
}

void TVideoWdg::useEdgeDetector(bool use)
{
    if (use) {
        if (!findMiddlewareByType<TEdgeDetector>()) {
            addMiddleware(new TEdgeDetector);
        }
    } else {
        removeMiddlewareByType<TEdgeDetector>();
    }
}

void TVideoWdg::addRTCPsource(QString url)
{
    rtcp_ = new TRTCPFrameProvider;
    fproviders_.append(rtcp_);
    rtcp_->setUrl(url.toStdString());
    auto rtcpReady = [this]() {
        videosrcDesc_.append(rtcp_->getDeviceDesc());
        emit videoSourcesChanged(videosrcDesc_);
    };
    rtcp_->start(rtcpReady);
}

void TVideoWdg::addMiddleware(IFrameMiddleware* middleware) {
    fmiddlewares_.push_back(std::unique_ptr<IFrameMiddleware>(middleware));
}

template<typename T>
void TVideoWdg::removeMiddlewareByType() {
    for (auto it = fmiddlewares_.end(); it != fmiddlewares_.begin();) {
        --it;
        if (dynamic_cast<T*>(it->get()) != nullptr) {
            it = fmiddlewares_.erase(it);
        }
    }
}

template<typename T>
bool TVideoWdg::findMiddlewareByType() {
    for (const auto& middleware : fmiddlewares_) {
        if (dynamic_cast<T*>(middleware.get()) != nullptr) {
            return true;
        }
    }
    return false;
}

void TVideoWdg::removeAllEdgeDetectors() {
    removeMiddlewareByType<TEdgeDetector>();
}

void TVideoWdg::updateVideoSize(const QImage& img)
{
    scene_->setSceneRect(0, 0, img.width(), img.height());
    img.width()/30 > 60 ? painter_->setFontSize(60) :  painter_->setFontSize(img.width()/30);
    img.width()/300 > 5 ? painter_->setLineWidth(5) :  painter_->setLineWidth(img.width()/300);
    resetTransform();
    scale(zoomFactor_, zoomFactor_);
}
