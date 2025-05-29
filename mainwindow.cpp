#include "mainwindow.h"
#include "./ui_mainwindow.h"

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
        currentDrawMode = checked ? Line : None;
        clearScene();
        isSettingCircleCenter = false;
    });

    QAction *actionDrawCircle = toolBar->addAction(
        QIcon(":/assets/icons/icons8-circle-50.png"),
        "Нарисовать круг"
        );
    actionDrawCircle->setCheckable(true);
    actionDrawCircle->setActionGroup(toolBarActGrp);
    connect(actionDrawCircle, &QAction::toggled, this, [this](bool checked) {
        currentDrawMode = checked ? Circle : None;
        clearScene();
        isSettingCircleCenter = checked;
    });
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
        // std::vector<std::vector<cv::Point> > contours;
        // std::vector<cv::Vec4i> hierarchy;
        // findContours( edges, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );

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

QImage MainWindow::Mat2QImage(const cv::Mat &mat)
{
    if (mat.empty()) return QImage();
    if (mat.type() == CV_8UC3) {
        QImage image(mat.cols, mat.rows, QImage::Format_RGB888);
        memcpy(image.bits(), mat.data, mat.total() * mat.elemSize());
        return image.rgbSwapped();
    } else if (mat.type() == CV_8UC1) {
        QImage image(mat.cols, mat.rows, QImage::Format_Grayscale8);
        memcpy(image.bits(), mat.data, mat.total() * mat.elemSize());
        return image;
    }
    return QImage();
}

cv::Mat MainWindow::QImage2Mat(const QImage &image)
{
    if (image.isNull()) return cv::Mat();
    if (image.format() == QImage::Format_RGB888) {
        cv::Mat mat(image.height(), image.width(), CV_8UC3);
        memcpy(mat.data, image.bits(), image.sizeInBytes());
        return mat;
    } else if (image.format() == QImage::Format_Grayscale8) {
        cv::Mat mat(image.height(), image.width(), CV_8UC1);
        memcpy(mat.data, image.bits(), image.sizeInBytes());
        return mat;
    }
    return cv::Mat();
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
    scene = new QGraphicsScene(this);
    ui->vidWgt->setScene(scene);

    pixmapItem = new QGraphicsPixmapItem();
    scene->addItem(pixmapItem);

    // Обработка событий мыши
    connect(ui->vidWgt, &TCameraViewWithPainter::mousePressed, this, [this](QMouseEvent *event) {
        if (currentDrawMode == None || event->button() != Qt::LeftButton) return;

        // Удаляем предыдущий элемент и его текст, если они существуют
        clearScene();

        // Удаляем временный элемент и его текст, если они остались
        if (tempItem && scene->items().contains(tempItem)) {
            scene->removeItem(tempItem);
            delete tempItem;
            tempItem = nullptr;
        }
        if (tempTextItem && scene->items().contains(tempTextItem)) {
            scene->removeItem(tempTextItem);
            delete tempTextItem;
            tempTextItem = nullptr;
        }

        if (currentDrawMode == Circle && isSettingCircleCenter) {
            // Фиксируем центр круга
            startPoint = ui->vidWgt->mapToScene(event->pos());
            isSettingCircleCenter = false;
            isDrawing = true;

            // Создаём временный круг для предпросмотра
            QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(0, 0, 0, 0);
            circle->setPen(QPen(Qt::blue, 1, Qt::DashLine));
            scene->addItem(circle);
            tempItem = circle;

            // Создаём временный текст
            tempTextItem = new QGraphicsTextItem("R: 0 px, 0 mm");
            tempTextItem->setFont(QFont("Arial", fontSize));
            tempTextItem->setDefaultTextColor(Qt::blue);
            scene->addItem(tempTextItem);
        } else if (currentDrawMode == Line || (currentDrawMode == Circle && !isSettingCircleCenter)) {
            // Начинаем рисование линии или радиуса круга
            startPoint = ui->vidWgt->mapToScene(event->pos());
            isDrawing = true;

            if (currentDrawMode == Line) {
                QGraphicsLineItem *line = new QGraphicsLineItem(startPoint.x(), startPoint.y(), startPoint.x(), startPoint.y());
                line->setPen(QPen(Qt::red, 1, Qt::DashLine));
                scene->addItem(line);
                tempItem = line;

                // Создаём временный текст
                tempTextItem = new QGraphicsTextItem("L: 0 px, 0 mm");
                tempTextItem->setFont(QFont("Arial", fontSize));
                tempTextItem->setDefaultTextColor(Qt::red);
                scene->addItem(tempTextItem);
            } else if (currentDrawMode == Circle) {
                QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(0, 0, 0, 0);
                circle->setPen(QPen(Qt::blue, 1, Qt::DashLine));
                scene->addItem(circle);
                tempItem = circle;

                // Создаём временный текст
                tempTextItem = new QGraphicsTextItem("R: 0 px, 0 mm");
                tempTextItem->setFont(QFont("Arial", fontSize));
                tempTextItem->setDefaultTextColor(Qt::blue);
                scene->addItem(tempTextItem);
            }
        }
    });

    connect(ui->vidWgt, &TCameraViewWithPainter::mouseMoved, this, [this](QMouseEvent *event) {
        if (currentDrawMode == None || !isDrawing) return;
        QPointF currentPoint = ui->vidWgt->mapToScene(event->pos());

        // Получаем коэффициенты перевода из пикселей в миллиметры
        qreal mmPerPixelWidth = ui->dsB_mmInPixelsWidth->value();
        qreal mmPerPixelHeight = ui->dsB_mmInPixelsHeight->value();

        // Проверяем модификаторы для линий
        Qt::KeyboardModifiers modifiers = event->modifiers();
        if (currentDrawMode == Line) {
            if (modifiers & Qt::ControlModifier) {
                currentPoint.setY(startPoint.y());
            } else if (modifiers & Qt::ShiftModifier) {
                currentPoint.setX(startPoint.x());
            }
        }

        // Обновляем временный элемент
        if (currentDrawMode == Line) {
            QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem*>(tempItem);
            if (line) {
                line->setLine(startPoint.x(), startPoint.y(), currentPoint.x(), currentPoint.y());
                if (tempTextItem) {
                    // Вычисляем длину линии в пикселях
                    qreal length = QLineF(startPoint, currentPoint).length();
                    // Вычисляем компоненты по X и Y
                    qreal dx = currentPoint.x() - startPoint.x();
                    qreal dy = currentPoint.y() - startPoint.y();
                    // Переводим в миллиметры
                    qreal dx_mm = dx * mmPerPixelWidth;
                    qreal dy_mm = dy * mmPerPixelHeight;
                    qreal lengthInmm = std::sqrt(dx_mm * dx_mm + dy_mm * dy_mm);
                    // Обновляем текст
                    tempTextItem->setFont(QFont("Arial", fontSize));
                    tempTextItem->setPlainText(QString("L: %1 px, %2 mm")
                                                   .arg(length, 0, 'f', 0)
                                                   .arg(lengthInmm, 0, 'f', 2));
                    // Позиционируем текст справа сверху
                    qreal textX = qMax(startPoint.x(), currentPoint.x()) + 5;
                    qreal textY = qMin(startPoint.y(), currentPoint.y()) - 5;
                    tempTextItem->setPos(textX, textY);
                }
                scene->update();
            }
        } else if (currentDrawMode == Circle) {
            QGraphicsEllipseItem *circle = qgraphicsitem_cast<QGraphicsEllipseItem*>(tempItem);
            if (circle) {
                // Вычисляем радиус в пикселях
                qreal radius = QLineF(startPoint, currentPoint).length();
                // Вычисляем компоненты радиуса по X и Y
                qreal dx = currentPoint.x() - startPoint.x();
                qreal dy = currentPoint.y() - startPoint.y();
                // Переводим в миллиметры
                qreal dx_mm = dx * mmPerPixelWidth;
                qreal dy_mm = dy * mmPerPixelHeight;
                qreal radiusInmm = std::sqrt(dx_mm * dx_mm + dy_mm * dy_mm);
                // Обновляем круг
                circle->setRect(startPoint.x() - radius, startPoint.y() - radius, radius * 2, radius * 2);
                if (tempTextItem) {
                    // Обновляем текст радиуса
                    tempTextItem->setFont(QFont("Arial", fontSize));
                    tempTextItem->setPlainText(QString("R: %1 px, %2 mm")
                                                   .arg(radius, 0, 'f', 0)
                                                   .arg(radiusInmm, 0, 'f', 2));
                    // Позиционируем текст справа сверху от круга
                    qreal textX = startPoint.x() + radius + 5;
                    qreal textY = startPoint.y() - radius - 5;
                    tempTextItem->setPos(textX, textY);
                }
                scene->update();
            }
        }
    });

    connect(ui->vidWgt, &TCameraViewWithPainter::mouseReleased, this, [this](QMouseEvent *event) {
        if (currentDrawMode == None || !isDrawing || event->button() != Qt::LeftButton) return;
        QPointF endPoint = ui->vidWgt->mapToScene(event->pos());

        // Получаем коэффициенты перевода из пикселей в миллиметры
        qreal mmPerPixelWidth = ui->dsB_mmInPixelsWidth->value();
        qreal mmPerPixelHeight = ui->dsB_mmInPixelsHeight->value();

        // Проверяем модификаторы для линий
        Qt::KeyboardModifiers modifiers = event->modifiers();
        if (currentDrawMode == Line) {
            if (modifiers & Qt::ControlModifier) {
                endPoint.setY(startPoint.y());
            } else if (modifiers & Qt::ShiftModifier) {
                endPoint.setX(startPoint.x());
            }
        }

        // Фиксируем конечный элемент
        if (currentDrawMode == Line) {
            // Удаляем временную линию и текст
            if (tempItem && scene->items().contains(tempItem)) {
                scene->removeItem(tempItem);
                delete tempItem;
                tempItem = nullptr;
            }
            if (tempTextItem && scene->items().contains(tempTextItem)) {
                scene->removeItem(tempTextItem);
                delete tempTextItem;
                tempTextItem = nullptr;
            }
            // Создаём постоянную линию
            QGraphicsLineItem *line = new QGraphicsLineItem(
                startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y()
                );
            line->setPen(QPen(Qt::red, 1));
            scene->addItem(line);
            lastItem = line;

            // Создаём постоянный текст
            qreal length = QLineF(startPoint, endPoint).length();
            qreal dx = endPoint.x() - startPoint.x();
            qreal dy = endPoint.y() - startPoint.y();
            qreal dx_mm = dx * mmPerPixelWidth;
            qreal dy_mm = dy * mmPerPixelHeight;
            qreal lengthInmm = std::sqrt(dx_mm * dx_mm + dy_mm * dy_mm);
            lastTextItem = new QGraphicsTextItem(QString("L: %1 px, %2 mm")
                                                     .arg(length, 0, 'f', 0)
                                                     .arg(lengthInmm, 0, 'f', 2));
            lastTextItem->setFont(QFont("Arial", fontSize));
            lastTextItem->setDefaultTextColor(Qt::red);
            qreal textX = qMax(startPoint.x(), endPoint.x()) + 5;
            qreal textY = qMin(startPoint.y(), endPoint.y()) - 5;
            lastTextItem->setPos(textX, textY);
            scene->addItem(lastTextItem);
        } else if (currentDrawMode == Circle) {
            // Удаляем временный круг и текст
            if (tempItem && scene->items().contains(tempItem)) {
                scene->removeItem(tempItem);
                delete tempItem;
                tempItem = nullptr;
            }
            if (tempTextItem && scene->items().contains(tempTextItem)) {
                scene->removeItem(tempTextItem);
                delete tempTextItem;
                tempTextItem = nullptr;
            }
            // Создаём постоянный круг
            qreal radius = QLineF(startPoint, endPoint).length();
            qreal dx = endPoint.x() - startPoint.x();
            qreal dy = endPoint.y() - startPoint.y();
            qreal dx_mm = dx * mmPerPixelWidth;
            qreal dy_mm = dy * mmPerPixelHeight;
            qreal radiusInmm = std::sqrt(dx_mm * dx_mm + dy_mm * dy_mm);
            QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(
                startPoint.x() - radius, startPoint.y() - radius, radius * 2, radius * 2
                );
            circle->setPen(QPen(Qt::blue, 1));
            scene->addItem(circle);
            lastItem = circle;

            // Создаём постоянный текст
            lastTextItem = new QGraphicsTextItem(QString("R: %1 px, %2 mm")
                                                     .arg(radius, 0, 'f', 0)
                                                     .arg(radiusInmm, 0, 'f', 2));
            lastTextItem->setFont(QFont("Arial", fontSize));
            lastTextItem->setDefaultTextColor(Qt::blue);
            qreal textX = startPoint.x() + radius + 5;
            qreal textY = startPoint.y() - radius - 5;
            lastTextItem->setPos(textX, textY);
            scene->addItem(lastTextItem);

            isSettingCircleCenter = true;
        }

        isDrawing = false;
    });
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
    setupGraphicsView();

    QMediaDevices mediaDevices;
    const QList<QCameraDevice> cameras = mediaDevices.videoInputs();
    if (cameras.isEmpty()) {
        statusBar()->showMessage(tr("No video input"));
    } else {
        for (const QCameraDevice &camera : cameras) {
            ui->cb_videoSources->addItem(camera.description());
        }
    }

    const QCameraDevice &cameraDevice = cameras.first();
    camera = new QCamera(cameraDevice, this);
    QList<QCameraFormat> formats = cameraDevice.videoFormats();
    for (const QCameraFormat &fmt : formats) {
        ui->cb_formats->addItem(QString("%1,%2").
                                arg(fmt.resolution().width()).
                                arg(fmt.resolution().height()));
    }
    captureSession->setCamera(camera);
    captureSession->setVideoSink(videoSink);
    camera->start();

    connect(videoSink, &QVideoSink::videoFrameChanged, this, &MainWindow::processFrame);
    connect(ui->cB_edgeDetection, &QCheckBox::stateChanged, this, &MainWindow::updateCannyState);

    connect(ui->cb_videoSources, &QComboBox::currentTextChanged, [this, cameras](const QString &text) {
        for (const QCameraDevice &cam : cameras) {
            if (text == cam.description()) {
                clearScene();
                camera->stop();
                camera = new QCamera(cam, this);
                ui->cb_formats->clear();
                QList<QCameraFormat> formats = cam.videoFormats();
                for (const QCameraFormat &fmt : formats) {
                    ui->cb_formats->addItem(QString("%1,%2").
                                            arg(fmt.resolution().width()).
                                            arg(fmt.resolution().height()));
                }
                if (!formats.isEmpty()) {
                    camera->setCameraFormat(formats.first());
                }
                captureSession->setCamera(camera);
                captureSession->setVideoSink(videoSink);
                captureSession->setVideoOutput(videoItem);
                camera->start();
                updateVideoSize();
            }
        }
    });
    connect(ui->cb_formats,&QComboBox::currentIndexChanged,[this, cameras](int index){
        if (index < 0) return;

        for (const QCameraDevice &cam : cameras) {
            if (ui->cb_videoSources->currentText() == cam.description()) {
                clearScene();
                camera->stop();
                camera = new QCamera(cam, this);
                QList<QCameraFormat> formats = cam.videoFormats();
                if (index >= formats.size()) {
                    return;
                }
                if (!formats.isEmpty()) {
                    camera->setCameraFormat(formats.at(index));
                    fontSize =formats.at(index).resolution().width()/300*10;
                }
                captureSession->setCamera(camera);
                captureSession->setVideoSink(videoSink);
                captureSession->setVideoOutput(videoItem);
                camera->start();
                updateVideoSize();
                break;
            }
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
