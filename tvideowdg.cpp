#include "tvideowdg.h"

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
        QImage img = fproviders_.at(currentActiveVideoProviderIdx_)->getFrame();
        currentFrame_->setPixmap(
            QPixmap{}.fromImage(
                img
            )
        );
        updateVideoSize(img);
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

void TVideoWdg::updateVideoSize(const QImage& img)
{
    scene_->setSceneRect(0, 0, img.width(), img.height());
    resetTransform();
    fitInView(currentFrame_, Qt::KeepAspectRatio);
}
