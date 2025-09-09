/** @file
 *
 * TVideoWdg - SimpleMeasurementByVideoTool
 * By Ilia.belou <ilia.belou@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TSURFACEPAINTER_H
#define TSURFACEPAINTER_H

#include <QObject>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsView>

constexpr int PX_DISPLAY_PRESICION = 0;                 ///< Precision for displaying pixel measurements (decimal places).
constexpr int UNITS_MES_DISPLAY_PRESICION = 2;          ///< Precision for displaying measurements in millimeters (decimal places).
constexpr int TEXT_DISPLAY_OFFSET_HOR_INPX = 5;         ///< Horizontal offset for text placement in pixels.
constexpr int TEXT_DISPLAY_OFFSET_VERT_INPX = 5;        ///< Vertical offset for text placement in pixels.

/*!
 * \class TSurfacePainter
 * \brief A class for drawing lines and circles on a QGraphicsScene with measurement annotations.
 *
 * The `TSurfacePainter` class is a `QObject` that facilitates interactive drawing of lines and circles on a
 * `QGraphicsScene`. It supports two drawing modes: `Line` and `Circle`, allowing users to draw shapes with
 * mouse interactions and display measurements in pixels and millimeters. The class handles preview and
 * permanent graphics items, updates annotations dynamically, and supports configuration of font size, line
 * width, and pixel-to-millimeter conversion factors. It is typically used in conjunction with a `QGraphicsView`
 * to visualize video frames or images with overlaid measurements.
 */
class TSurfacePainter : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Enumeration for drawing modes.
     */
    enum class DrawMode : uint {
        None,   ///< No drawing mode (inactive).
        Line,   ///< Draw a line segment.
        Circle  ///< Draw a circle.
    };

    /*!
     * \brief Constructs a TSurfacePainter instance.
     * \param scene Pointer to the QGraphicsScene where drawing occurs (default is nullptr).
     *
     * Initializes the painter with the provided scene. The scene is not owned by the painter.
     */
    explicit TSurfacePainter(QGraphicsScene* scene = nullptr);

    /*!
     * \brief Destructor.
     *
     * Cleans up all temporary and permanent graphics items and text annotations from the scene.
     */
    ~TSurfacePainter();
public slots:    
    /*!
     * \brief Handles mouse press events to start drawing.
     * \param event The mouse event containing press details.
     *
     * Initiates drawing of a line or circle based on the current draw mode. For circles, it supports
     * setting the center point if `isSettingCircleCenter_` is true.
     */
    void handleMousePressed(QMouseEvent *event);

    /*!
     * \brief Handles mouse move events to update drawing in progress.
     * \param event The mouse event containing movement details.
     *
     * Updates the temporary graphics item (line or circle) and its measurement annotation based on
     * the current mouse position. Supports horizontal/vertical constraints for lines with Ctrl/Shift modifiers.
     */
    void handleMouseMoved(QMouseEvent *event);

    /*!
     * \brief Handles mouse release events to finalize drawing.
     * \param event The mouse event containing release details.
     *
     * Finalizes the drawing by creating a permanent graphics item and its measurement annotation.
     * For circles, it resets the center-setting mode.
     */
    void handleMouseReleased(QMouseEvent *event);

    /*!
     * \brief Clears all permanent graphics items and text annotations from the scene.
     */
    void clearScene();

    /*!
     * \brief Sets the font size for measurement annotations.
     * \param size The font size in points (clamped between 1 and 60).
     */
    void setFontSize(int size);

    /*!
     * \brief Sets the line width for drawn shapes.
     * \param width The line width in pixels (clamped between 1 and 5).
     */
    void setLineWidth(int width);

    /*!
     * \brief Sets the conversion factor from pixels to millimeters for width measurements.
     * \param value The millimeters per pixel (minimum 0.001).
     */
    void setmmInPixelsWidth(double value);

    /*!
     * \brief Sets the conversion factor from pixels to millimeters for height measurements.
     * \param value The millimeters per pixel (minimum 0.001).
     */
    void setmmInPixelsHeight(double value);

    /*!
     * \brief Sets the current drawing mode.
     * \param drawMode The drawing mode (None, Line, or Circle).
     */
    void setCurrentDrawMode(DrawMode drawMode);

    /*!
     * \brief Sets whether the next mouse press sets the circle's center.
     * \param flag If true, the next press sets the circle center; otherwise, it starts radius drawing.
     */
    void setSettingCircleCenter(bool flag);
private:
    QGraphicsScene* scene_;                             ///< Pointer to the scene (ownership by parent).
    DrawMode currentDrawMode_ = DrawMode::None;         ///< Current drawing mode.
    QPointF startPoint_;                                ///< Starting point for the current drawing.
    QGraphicsItem* tempItem_ = nullptr;                 ///< Temporary graphics item for drawing preview.
    QGraphicsItem* lastItem_ = nullptr;                 ///< Last permanent graphics item drawn.
    QList<QGraphicsItem*> paintedObjInScene;            ///< List of permanent graphics items in the scene.
    QGraphicsTextItem* tempTextItem_ = nullptr;         ///< Temporary text item for measurement preview.
    QGraphicsTextItem* lastTextItem_ = nullptr;         ///< Last permanent text item for measurements.
    QList<QGraphicsTextItem*> paintedTextObjInScene;    ///< List of permanent text items in the scene.
    bool isSettingCircleCenter_ = false;                ///< Flag indicating if the next press sets the circle center.
    bool isDrawing_ = false;                            ///< Flag indicating if drawing is in progress.
    int fontSize_ = 10;                                 ///< Font size for measurement annotations.
    int lineWidth_ = 1;                                 ///< Line width for drawn shapes.
    double mmInPixelsWidth_ = 0.001;                    ///< Millimeters per pixel for width measurements.
    double mmInPixelsHeight_ = 0.001;                    ///< Millimeters per pixel for height measurements.

    /*!
     * \brief Removes temporary graphics and text items from the scene.
     */
    void clearTempObjs();

    /*!
     * \brief Calculates the radius of a circle in millimeters.
     * \param centreInPx The center point of the circle in pixels.
     * \param rPointInPx A point on the circle's circumference in pixels.
     * \return The radius in millimeters.
     */
    double calculateCircleRadiusInMm(const QPointF& centreInPx, const QPointF& rPointInPx) const;

    /*!
     * \brief Calculates the length of a line segment in millimeters.
     * \param startPointInPx The start point of the line in pixels.
     * \param endPointInPx The end point of the line in pixels.
     * \return The length in millimeters.
     */
    double calculateLineLengthInMm(const QPointF& startPointInPx, const QPointF& endPointInPx) const;
};

#endif // TSURFACEPAINTER_H
