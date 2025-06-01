#include "tvideowdg.h"
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
        currentFrame_->setPixmap(
            QPixmap{}.fromImage(
                currentFrameImg_
            )
        );
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
    // Увеличиваем масштаб на фиксированную величину
    zoomFactor_ += ZOOM_FACTOR;
    // Ограничиваем максимальный масштаб до 1.0 (100%)
    if (zoomFactor_ > 1.0) {
        zoomFactor_ = 1.0;
    }
    // Сбрасываем текущую трансформацию
    resetTransform();
    // Применяем новый масштаб
    scale(zoomFactor_, zoomFactor_);
    // Обновляем отображение, сохраняя пропорции
    fitInView(currentFrame_, Qt::KeepAspectRatio);
    // Обновляем сцену
    scene_->update();
}

void TVideoWdg::decZoom()
{
    // Уменьшаем масштаб на фиксированную величину
    zoomFactor_ -= ZOOM_FACTOR;
    // Ограничиваем минимальный масштаб до 0.1 (10%)
    if (zoomFactor_ < 0.05) {
        zoomFactor_ = 0.05;
    }
    // Сбрасываем текущую трансформацию
    resetTransform();
    // Применяем новый масштаб
    scale(zoomFactor_, zoomFactor_);
    // Обновляем отображение, сохраняя пропорции
    fitInView(currentFrame_, Qt::KeepAspectRatio);
    // Обновляем сцену
    scene_->update();
}

void TVideoWdg::fit()
{
    scene_->setSceneRect(0, 0, currentFrameImg_.width(), currentFrameImg_.height());
    zoomFactor_ = 1.0;
    resetTransform();
    fitInView(currentFrame_, Qt::KeepAspectRatio);
}

void TVideoWdg::updateVideoSize(const QImage& img)
{
    scene_->setSceneRect(0, 0, img.width(), img.height());
    img.width()/30 > 60 ? painter_->setFontSize(60) :  painter_->setFontSize(img.width()/30);
    img.width()/300 > 5 ? painter_->setLineWidth(5) :  painter_->setLineWidth(img.width()/300);
    resetTransform();
    scale(zoomFactor_, zoomFactor_);
}
