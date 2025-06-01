#include "mainwindow.h"
#include "./ui_mainwindow.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , camera(nullptr)
    , captureSession(new QMediaCaptureSession(this))
    , currentDrawMode(None)
    , videoSink(new QVideoSink(this))
    , pixmapItem(nullptr)
{
    ui->setupUi(this);
    initializeToolBar();

    connect(ui->vidWgt, &TVideoWdg::videoSourcesChanged, [this](QList<std::string> srcs) {
        QSignalBlocker blocker(ui->cb_videoSources);
        ui->cb_videoSources->clear();
        foreach (std::string str, srcs) {
            ui->cb_videoSources->addItem(QString::fromStdString(str));
        }
    });
    connect(ui->vidWgt, &TVideoWdg::videoFormatsChanged, [this](QList<std::string> srcs) {
        QSignalBlocker blocker(ui->cb_formats);
        ui->cb_formats->clear();
        foreach (std::string str, srcs) {
            ui->cb_formats->addItem(QString::fromStdString(str));
        }
    });

    // Disable lE_videoSourceUrl by default
    ui->lE_videoSourceUrl->setEnabled(false);

    // Connect video source selection
    connect(ui->cb_videoSources, &QComboBox::currentTextChanged, ui->vidWgt, &TVideoWdg::changeVideoSrc);

    // Connect RTSP URL changes
    //connect(ui->lE_videoSourceUrl, &QLineEdit::textChanged, ui->vidWgt, &TVideoWdg::videoFormatsChanged);

    // Connect format selection
    connect(ui->cb_formats, &QComboBox::currentIndexChanged, ui->vidWgt, &TVideoWdg::changeVideofmt);

    // Отрисовка граф.элементов / painter
    connect(ui->dsB_mmInPixelsHeight,&QDoubleSpinBox::valueChanged,ui->vidWgt->getPainter(),&TSurfacePainter::setmmInPixelsHeight);
    connect(ui->dsB_mmInPixelsWidth,&QDoubleSpinBox::valueChanged,ui->vidWgt->getPainter(),&TSurfacePainter::setmmInPixelsWidth);

    //Обнаружение границ / Edge Detector
    connect(ui->cB_edgeDetection,&QCheckBox::clicked,ui->vidWgt,&TVideoWdg::useEdgeDetector);
}

void MainWindow::initializeToolBar()
{
    QToolBar *toolBar = this->addToolBar("Панель инструментов");
    QActionGroup *toolBarActGrp = new QActionGroup(this);
    toolBarActGrp->setExclusive(true);

    QAction *actionDrawLine = toolBar->addAction(
        QIcon(":/assets/icons/icons8-line-50.png"),
        "Нарисовать линию"
        );
    actionDrawLine->setCheckable(true);
    actionDrawLine->setActionGroup(toolBarActGrp);
    connect(actionDrawLine, &QAction::toggled, this, [this](bool checked) {
        TSurfacePainter::DrawMode mode = checked ? TSurfacePainter::DrawMode::Line : TSurfacePainter::DrawMode::None;
        ui->vidWgt->getPainter()->setSettingCircleCenter(false);
        ui->vidWgt->getPainter()->setCurrentDrawMode(mode);
    });

    QAction *actionDrawCircle = toolBar->addAction(
        QIcon(":/assets/icons/icons8-circle-50.png"),
        "Нарисовать круг"
        );
    actionDrawCircle->setCheckable(true);
    actionDrawCircle->setActionGroup(toolBarActGrp);
    connect(actionDrawCircle, &QAction::toggled, this, [this](bool checked) {
        TSurfacePainter::DrawMode mode = checked ? TSurfacePainter::DrawMode::Circle : TSurfacePainter::DrawMode::None;
        ui->vidWgt->getPainter()->setSettingCircleCenter(checked);
        ui->vidWgt->getPainter()->setCurrentDrawMode(mode);
    });

    QAction *actionClearScene = toolBar->addAction(
        QIcon(":/assets/icons/eraser.png"),
        "Очистить сцену"
        );
    connect(actionClearScene, &QAction::triggered, this, [this](bool state) {
        ui->vidWgt->getPainter()->clearScene();
    });

    QAction *actionZoomPlus = toolBar->addAction(
        QIcon(":/assets/icons/zoom-in.png"),
        "Увеличить"
        );
    connect(actionZoomPlus, &QAction::triggered, ui->vidWgt, &TVideoWdg::incZoom);

    QAction *actionZoomMinus = toolBar->addAction(
        QIcon(":/assets/icons/zoom-out.png"),
        "Уменьшить"
        );
    connect(actionZoomMinus, &QAction::triggered, ui->vidWgt, &TVideoWdg::decZoom);

    QAction *actionFit = toolBar->addAction(
        QIcon(":/assets/icons/fit.png"),
        "Вписать"
        );
    connect(actionFit, &QAction::triggered, ui->vidWgt, &TVideoWdg::fit);
}

void MainWindow::processFrame(const QVideoFrame &frame)
{
    QVideoFrame videoFrame = frame;
    if (!videoFrame.isValid() || !videoFrame.map(QVideoFrame::ReadOnly)) {
        qDebug() << "Invalid or unmappable video frame";
        return;
    }

    QImage image = videoFrame.toImage();
    videoFrame.unmap();

    if (image.isNull()) {
        qDebug() << "Null QImage";
        return;
    }

    qDebug() << "QImage format:" << image.format() << "size:" << image.size();

    // Convert and copy QImage to ensure stable memory
    if (image.format() != QImage::Format_RGB32 && image.format() != QImage::Format_ARGB32) {
        qDebug() << "Converting QImage to Format_RGB32";
        image = image.convertToFormat(QImage::Format_RGB32).copy(); // Deep copy
        if (image.isNull()) {
            qDebug() << "Failed to convert QImage to Format_RGB32";
            return;
        }
    }

    //cv::Mat mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
    cv::Mat mat = QtOcv::image2Mat(image);
    if (mat.empty()) {
        qDebug() << "Empty cv::Mat";
        return;
    }
    qDebug() << "cv::Mat size:" << mat.cols << "x" << mat.rows << "type:" << mat.type();

    cv::Mat processed = mat.clone();
    if (processed.empty()) {
        qDebug() << "Empty processed cv::Mat";
        return;
    }


    if (enableCanny) {
        cv::Mat gray;
        qDebug() << "Before cvtColor: processed size:" << processed.cols << "x" << processed.rows << "type:" << processed.type();
        try {
            cv::cvtColor(processed, gray, cv::COLOR_BGR2GRAY);
        } catch (const cv::Exception& e) {
            qDebug() << "cv::cvtColor failed:" << e.what();
            return;
        }
        if (gray.empty()) {
            qDebug() << "Empty gray cv::Mat after cvtColor";
            return;
        }
        qDebug() << "Gray size:" << gray.cols << "x" << gray.rows << "type:" << gray.type();

        cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);
        cv::Mat edges;
        cv::Canny(gray, edges, ui->dSb_edgeDetectionThr1->value(), ui->dSb_edgeDetectionThr2->value());
        cv::cvtColor(edges, processed, cv::COLOR_GRAY2BGRA);
    }

    QImage outputImage((const uchar*)processed.data, processed.cols, processed.rows, processed.step, QImage::Format_RGB32);
    outputImage = outputImage.copy();
    if (outputImage.isNull()) {
        qDebug() << "Null output QImage";
        return;
    }

    // Ensure GUI updates are thread-safe
    QMetaObject::invokeMethod(this, [=]() {
        pixmapItem->setPixmap(QPixmap::fromImage(outputImage));
        scene->setSceneRect(0, 0, outputImage.width(), outputImage.height());
    }, Qt::QueuedConnection);
}

void MainWindow::updateCannyState(int state)
{
    enableCanny = (state == Qt::Checked);
}

void MainWindow::clearScene()
{
    if (scene == nullptr) {
        return;
    }
    if (lastItem && scene->items().contains(lastItem)) {
        scene->removeItem(lastItem);
        delete lastItem;
        lastItem = nullptr;
    }
    if (lastTextItem && scene->items().contains(lastTextItem)) {
        scene->removeItem(lastTextItem);
        delete lastTextItem;
        lastTextItem = nullptr;
    }
}

void MainWindow::setupGraphicsView()
{

}

void MainWindow::updateVideoSize()
{
    if (!pixmapItem || !camera || !camera->isActive()) return;

    QSize videoSize = camera->cameraFormat().resolution();
    if (!videoSize.isValid()) return;

    scene->setSceneRect(0, 0, videoSize.width(), videoSize.height());
    ui->vidWgt->resetTransform();
    ui->vidWgt->fitInView(pixmapItem, Qt::KeepAspectRatio);
}

void MainWindow::startRtspCapture(const QString &url)
{
    stopRtspCapture(); // Clean up any existing RTSP capture

    rtspCapture = new cv::VideoCapture(url.toStdString(), cv::CAP_FFMPEG);
    if (!rtspCapture->isOpened()) {
        qDebug() << "Failed to open RTSP stream:" << url;
        delete rtspCapture;
        rtspCapture = nullptr;
        return;
    }

    rtspTimer = new QTimer(this);
    connect(rtspTimer, &QTimer::timeout, this, &MainWindow::processRtspFrame,Qt::QueuedConnection);
    rtspTimer->start(33);
}

void MainWindow::stopRtspCapture()
{
    if (rtspTimer) {
        rtspTimer->stop();
        delete rtspTimer;
        rtspTimer = nullptr;
    }
    if (rtspCapture) {
        rtspCapture->release();
        delete rtspCapture;
        rtspCapture = nullptr;
    }
}

void MainWindow::processRtspFrame()
{
    if (!rtspCapture || !rtspCapture->isOpened()) {
        qDebug() << "RTSP capture not opened";
        stopRtspCapture();
        return;
    }

    cv::Mat frame;
    if (!rtspCapture->read(frame)) {
        qDebug() << "Failed to read RTSP frame";
        return;
    }

    if (frame.empty()) {
        qDebug() << "Empty RTSP frame";
        return;
    }

    // Convert to BGRA for consistency
    cv::Mat processed;
    if (frame.type() == CV_8UC3) {
        cv::cvtColor(frame, processed, cv::COLOR_BGR2BGRA);
    } else if (frame.type() == CV_8UC4) {
        processed = frame.clone();
    } else {
        qDebug() << "Unsupported RTSP frame type:" << frame.type();
        return;
    }

    if (enableCanny) {
        cv::setNumThreads(0);
        cv::Mat gray(processed.rows, processed.cols, CV_8U);
        try {
            cv::cvtColor(processed, gray, cv::COLOR_BGRA2GRAY);
        } catch (const cv::Exception& e) {
            qDebug() << "cv::cvtColor failed for RTSP:" << e.what();
            return;
        }
        cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);
        cv::Mat edges;
        cv::Canny(gray, edges, ui->dSb_edgeDetectionThr1->value(), ui->dSb_edgeDetectionThr2->value());
        cv::cvtColor(edges, processed, cv::COLOR_GRAY2BGRA);
    }

    QImage outputImage((const uchar*)processed.data, processed.cols, processed.rows, processed.step, QImage::Format_RGB32);
    outputImage = outputImage.copy();
    if (outputImage.isNull()) {
        qDebug() << "Null RTSP output QImage";
        return;
    }

    QMetaObject::invokeMethod(this, [=]() {
        pixmapItem->setPixmap(QPixmap::fromImage(outputImage));
        scene->setSceneRect(0, 0, outputImage.width(), outputImage.height());
    }, Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}
