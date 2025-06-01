#include "mainwindow.h"
#include "./ui_mainwindow.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
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

    // Connect video source selection
    connect(ui->cb_videoSources, &QComboBox::currentTextChanged, ui->vidWgt, &TVideoWdg::changeVideoSrc);

    // Connect RTSP URL changes
    connect(ui->pB_addRTCPurl, &QPushButton::clicked,[this]() {
        ui->vidWgt->addRTCPsource(ui->lE_videoSourceUrl->text());
    });

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

MainWindow::~MainWindow()
{
    delete ui;
}
