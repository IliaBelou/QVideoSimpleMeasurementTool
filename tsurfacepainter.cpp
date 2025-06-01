#include "tsurfacepainter.h"
#include <qgraphicsview.h>

TSurfacePainter::TSurfacePainter(QGraphicsScene *scene_) : scene_(scene_)
{

}

void TSurfacePainter::handleMousePressed(QMouseEvent *event)
{
    if (currentDrawMode_ == DrawMode::None || event->button() != Qt::LeftButton) return;
    if (scene_ == nullptr) return;

    // Удаляем временный элемент и его текст, если они остались
    clearTempObjs();

    auto createTempText = [this](Qt::GlobalColor color) {
        tempTextItem_ = new QGraphicsTextItem("R: 0 px, 0 mm");
        tempTextItem_->setFont(QFont("Arial", fontSize_));
        tempTextItem_->setDefaultTextColor(color);
    };

    switch (currentDrawMode_) {
    case DrawMode::Circle:
        if (isSettingCircleCenter_) {
            // Фиксируем центр круга
            if (!scene_->views().isEmpty()) {
                startPoint_ = scene_->views().first()->mapToScene(event->pos());
            }
            isSettingCircleCenter_ = false;
            isDrawing_ = true;

            // Временный круг для предпросмотра
            QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(0, 0, 0, 0);
            circle->setPen(QPen(Qt::blue, lineWidth_, Qt::DashLine));
            scene_->addItem(circle);
            tempItem_ = circle;

            // Временный текст дял предпросмотра
            createTempText(Qt::blue);
            scene_->addItem(tempTextItem_);
        }else{
            // Начинаем рисование радиуса круга
            if (!scene_->views().isEmpty()) {
                startPoint_ = scene_->views().first()->mapToScene(event->pos());
            }
            isDrawing_ = true;
            QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(0, 0, 0, 0);
            circle->setPen(QPen(Qt::blue, 1, Qt::DashLine));
            scene_->addItem(circle);
            tempItem_ = circle;

            // Создаём временный текст
            createTempText(Qt::blue);
            scene_->addItem(tempTextItem_);
        }
        break;
    case DrawMode::Line:
    {
        // Начинаем рисование линии
        if (!scene_->views().isEmpty()) {
            startPoint_ = scene_->views().first()->mapToScene(event->pos());
        }
        isDrawing_ = true;
        QGraphicsLineItem *line = new QGraphicsLineItem(startPoint_.x(), startPoint_.y(), startPoint_.x(), startPoint_.y());
        line->setPen(QPen(Qt::red, 1, Qt::DashLine));
        scene_->addItem(line);
        tempItem_ = line;

        // Создаём временный текст
        createTempText(Qt::red);
        scene_->addItem(tempTextItem_);
    }
    break;
    case DrawMode::None:break;
    }
}

void TSurfacePainter::handleMouseMoved(QMouseEvent *event)
{
    if (currentDrawMode_ == DrawMode::None || !isDrawing_) return;
    if (scene_ == nullptr) return;

    QPointF currentPoint;
    if (!scene_->views().isEmpty()) {
        currentPoint = scene_->views().first()->mapToScene(event->pos());
    }

    // CTRL - рисуем горизонтально, SHIFT - рисуем вертикально
    Qt::KeyboardModifiers modifiers = event->modifiers();
    if (currentDrawMode_ == DrawMode::Line) {
        if (modifiers & Qt::ControlModifier) {
            currentPoint.setY(startPoint_.y());
        } else if (modifiers & Qt::ShiftModifier) {
            currentPoint.setX(startPoint_.x());
        }
    }

    // Обновляем временный элемент
    switch (currentDrawMode_) {
    case DrawMode::Line:
    {
        QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem*>(tempItem_);
        if (line) {
            line->setLine(startPoint_.x(), startPoint_.y(), currentPoint.x(), currentPoint.y());
            if (tempTextItem_) {
                double lengthInpx = QLineF(startPoint_, currentPoint).length();
                double lengthInmm = lineLenghtInmm(startPoint_,currentPoint);
                // Обновляем текст
                tempTextItem_->setFont(QFont("Arial", fontSize_));
                tempTextItem_->setPlainText(QString("L: %1 px, %2 mm")
                                                .arg(lengthInpx, 0, 'f', PX_DISPLAY_PRESICION)
                                                .arg(lengthInmm, 0, 'f', UNITS_MES_DISPLAY_PRESICION));
                // Позиционируем текст справа сверху
                double textX = qMax(startPoint_.x(), currentPoint.x()) + TEXT_DISPLAY_OFFSET_HOR_INPX;
                double textY = qMin(startPoint_.y(), currentPoint.y()) - TEXT_DISPLAY_OFFSET_VERT_INPX;
                tempTextItem_->setPos(textX, textY);
            }
            scene_->update();
        }
    }
    break;
    case DrawMode::Circle:
    {
        QGraphicsEllipseItem *circle = qgraphicsitem_cast<QGraphicsEllipseItem*>(tempItem_);
        if (circle) {
            double radiusInPx = QLineF(startPoint_, currentPoint).length();
            double radiusdx = currentPoint.x() - startPoint_.x();
            double radiousdy = currentPoint.y() - startPoint_.y();
            double radiusdx_mm = radiusdx * mmInPixelsWidth_;
            double radiusdy_mm = radiousdy * mmInPixelsHeight_;
            double radiusInmm = std::sqrt(radiusdx_mm * radiusdx_mm + radiusdy_mm * radiusdy_mm);
            // Обновляем круг
            circle->setRect(startPoint_.x() - radiusInPx, startPoint_.y() - radiusInPx, radiusInPx * 2, radiusInPx * 2);
            if (tempTextItem_) {
                // Текст радиуса
                tempTextItem_->setFont(QFont("Arial", fontSize_));
                tempTextItem_->setPlainText(QString("R: %1 px, %2 mm")
                                                .arg(radiusInPx, 0, 'f', PX_DISPLAY_PRESICION)
                                                .arg(radiusInmm, 0, 'f', UNITS_MES_DISPLAY_PRESICION));
                // Позиционируем текст справа сверху от круга
                double textX = startPoint_.x() + radiusInPx + TEXT_DISPLAY_OFFSET_HOR_INPX;
                double textY = startPoint_.y() - radiusInPx - TEXT_DISPLAY_OFFSET_VERT_INPX;
                tempTextItem_->setPos(textX, textY);
            }
            scene_->update();
        }
    }
    break;
    case DrawMode::None: break;
    }
}

void TSurfacePainter::handleMouseReleased(QMouseEvent *event)
{
    if (currentDrawMode_ == DrawMode::None || !isDrawing_ || event->button() != Qt::LeftButton) return;
    QPointF endPoint;
    if (!scene_->views().isEmpty()) {
        endPoint = scene_->views().first()->mapToScene(event->pos());
    }

    // Модификаторы для линий
    Qt::KeyboardModifiers modifiers = event->modifiers();
    if (currentDrawMode_ == DrawMode::Line) {
        if (modifiers & Qt::ControlModifier) {
            endPoint.setY(startPoint_.y());
        } else if (modifiers & Qt::ShiftModifier) {
            endPoint.setX(startPoint_.x());
        }
    }

    // Фиксируем конечный элемент
    if (currentDrawMode_ == DrawMode::Line) {
        // Удаляем временную линию и текст
        clearTempObjs();

        // Создаём постоянную линию
        QGraphicsLineItem *line = new QGraphicsLineItem(
            startPoint_.x(), startPoint_.y(), endPoint.x(), endPoint.y()
            );
        line->setPen(QPen(Qt::red, 1));
        scene_->addItem(line);
        lastItem_ = line;
        paintedObjInScene.append(lastItem_);
        // Создаём постоянный текст
        double length = QLineF(startPoint_, endPoint).length();
        double lengthInmm = lineLenghtInmm(startPoint_,endPoint);
        lastTextItem_ = new QGraphicsTextItem(QString("L: %1 px, %2 mm")
                                                  .arg(length, 0, 'f', PX_DISPLAY_PRESICION)
                                                  .arg(lengthInmm, 0, 'f', UNITS_MES_DISPLAY_PRESICION));
        lastTextItem_->setFont(QFont("Arial", fontSize_));
        lastTextItem_->setDefaultTextColor(Qt::red);
        double textX = qMax(startPoint_.x(), endPoint.x()) + TEXT_DISPLAY_OFFSET_HOR_INPX;
        double textY = qMin(startPoint_.y(), endPoint.y()) - TEXT_DISPLAY_OFFSET_VERT_INPX;
        lastTextItem_->setPos(textX, textY);
        scene_->addItem(lastTextItem_);
        paintedTextObjInScene.append(lastTextItem_);
    } else if (currentDrawMode_ == DrawMode::Circle) {
        // Удаляем временный круг и текст
        if (tempItem_ && scene_->items().contains(tempItem_)) {
            scene_->removeItem(tempItem_);
            delete tempItem_;
            tempItem_ = nullptr;
        }
        if (tempTextItem_ && scene_->items().contains(tempTextItem_)) {
            scene_->removeItem(tempTextItem_);
            delete tempTextItem_;
            tempTextItem_ = nullptr;
        }
        // Создаём постоянный круг
        qreal radius = QLineF(startPoint_, endPoint).length();
        qreal dx = endPoint.x() - startPoint_.x();
        qreal dy = endPoint.y() - startPoint_.y();
        qreal dx_mm = dx * mmInPixelsWidth_;
        qreal dy_mm = dy * mmInPixelsHeight_;
        qreal radiusInmm = std::sqrt(dx_mm * dx_mm + dy_mm * dy_mm);
        QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(
            startPoint_.x() - radius, startPoint_.y() - radius, radius * 2, radius * 2
            );
        circle->setPen(QPen(Qt::blue, 1));
        scene_->addItem(circle);
        lastItem_ = circle;
        paintedObjInScene.append(lastItem_);
        // Создаём постоянный текст
        lastTextItem_ = new QGraphicsTextItem(QString("R: %1 px, %2 mm")
                                                  .arg(radius, 0, 'f', 0)
                                                  .arg(radiusInmm, 0, 'f', 2));
        lastTextItem_->setFont(QFont("Arial", fontSize_));
        lastTextItem_->setDefaultTextColor(Qt::blue);
        qreal textX = startPoint_.x() + radius + 5;
        qreal textY = startPoint_.y() - radius - 5;
        lastTextItem_->setPos(textX, textY);
        scene_->addItem(lastTextItem_);
        paintedTextObjInScene.append(lastTextItem_);
        isSettingCircleCenter_ = true;
    }

    isDrawing_ = false;
}

void TSurfacePainter::clearScene()
{
    if (scene_ == nullptr) {
        return;
    }
    foreach (QGraphicsItem* obj, paintedObjInScene) {
        if (obj && scene_->items().contains(obj)) {
            scene_->removeItem(obj);
            delete obj;
        }
    }
    foreach (QGraphicsTextItem* objText, paintedTextObjInScene) {
        if (objText && scene_->items().contains(objText)) {
            scene_->removeItem(objText);
            delete objText;
        }
    }
    lastItem_ = nullptr;
    lastTextItem_ = nullptr;
}

void TSurfacePainter::setFontSize(int size)
{
    size > 60 ? size = 60 : false;
    size < 1 ? size = 1 : false;
    fontSize_ = size;
}

void TSurfacePainter::setLineWidth(int width)
{
    width > 5 ? width = 5 : false;
    width < 1 ? width = 1 : false;
    lineWidth_ = width;
}

void TSurfacePainter::setmmInPixelsWidth(double value)
{
    value < 0 ? value = 0.001 : false;
    mmInPixelsWidth_ = value;
}

void TSurfacePainter::setmmInPixelsHeight(double value)
{
    value < 0 ? value = 0.001 : false;
    mmInPixelsHeight_ = value;
}

void TSurfacePainter::setCurrentDrawMode(DrawMode drawMode)
{
    switch (drawMode) {
    case DrawMode::Line:
        currentDrawMode_ = DrawMode::Line;
        break;
    case DrawMode::Circle:
        currentDrawMode_ = DrawMode::Circle;
        break;
    default:
        currentDrawMode_ = DrawMode::None;
    }
}

void TSurfacePainter::setSettingCircleCenter(bool flag)
{
    isSettingCircleCenter_ = flag;
}

void TSurfacePainter::clearTempObjs()
{
    if (tempItem_ && scene_->items().contains(tempItem_)) {
        scene_->removeItem(tempItem_);
        delete tempItem_;
        tempItem_ = nullptr;
    }
    if (tempTextItem_ && scene_->items().contains(tempTextItem_)) {
        scene_->removeItem(tempTextItem_);
        delete tempTextItem_;
        tempTextItem_ = nullptr;
    }
}

double TSurfacePainter::lineLenghtOfCircleInmm(QPointF centreInPx, QPointF rPointInPx)
{
    // Вычисляем компоненты по X и Y
    double dx = rPointInPx.x() - centreInPx.x();
    double dy = rPointInPx.y() - centreInPx.y();
    // Переводим в миллиметры
    double dx_mm = dx * mmInPixelsWidth_;
    double dy_mm = dy * mmInPixelsHeight_;
    double lengthInmm = std::sqrt(dx_mm * dx_mm + dy_mm * dy_mm);
    return lengthInmm;
}

double TSurfacePainter::lineLenghtInmm(QPointF centreInPx, QPointF rPointInPx)
{
    // Вычисляем компоненты по X и Y
    double dx = rPointInPx.x() - centreInPx.x();
    double dy = rPointInPx.y() - centreInPx.y();
    // Переводим в миллиметры
    double dx_mm = dx * mmInPixelsWidth_;
    double dy_mm = dy * mmInPixelsHeight_;
    double lengthInmm = std::sqrt(dx_mm * dx_mm + dy_mm * dy_mm);
    return lengthInmm;
}

