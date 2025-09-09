/** @file
 *
 * TVideoWdg - SimpleMeasurementByVideoTool
 * By Ilia.belou <ilia.belou@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TVIDEOWDG_H
#define TVIDEOWDG_H

#include <QGraphicsView>
#include <QObject>
#include <QTimer>
#include <QPixmap>

#include "video_wdg/surface_painter/tsurfacepainter.h"
#include "video_wdg/frame_middleware/iframemiddleware.h"
#include "video_wdg/frame_providers/iframeprovider.h"

constexpr int FRAME_UPDATE_PERIOD = 50; ///< Frame update period in milliseconds.
constexpr double ZOOM_FACTOR = 0.05;    ///< Zoom increment/decrement factor per step.

/*!
 * \class TVideoWdg
 * \brief A widget for displaying and manipulating video streams with support for frame processing and zooming.
 *
 * The `TVideoWdg` class is a custom `QGraphicsView` widget designed to display video frames from various sources
 * (e.g., USB devices or RTSP streams) and apply middleware processing (e.g., edge detection). It supports zooming,
 * fitting the video to the view, and handling mouse interactions for surface painting. The widget uses a timer to
 * periodically update frames and emits signals for user interactions and video source/format changes.
 */
class TVideoWdg : public QGraphicsView
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a TVideoWdg widget.
     * \param parent The parent widget (default is nullptr).
     *
     * Initializes the graphics scene, surface painter, frame providers, and timer for frame updates.
     * Connects to a default USB video device provider and starts periodic frame updates.
     */
    TVideoWdg(QWidget *parent = nullptr);

    /*!
     * \brief Destructor.
     *
     * Cleans up resources, including the surface painter and frame providers.
     */
    ~TVideoWdg();

    /*!
     * \brief Retrieves the surface painter associated with the widget.
     * \return Pointer to the TSurfacePainter instance.
     */
    TSurfacePainter* getPainter();
signals:
    /*!
     * \brief Emitted when the mouse is pressed on the widget.
     * \param event The mouse event containing details of the press.
     */
    void mousePressed(QMouseEvent *event);

    /*!
     * \brief Emitted when the mouse is moved within the widget.
     * \param event The mouse event containing movement details.
     */
    void mouseMoved(QMouseEvent *event);

    /*!
     * \brief Emitted when the mouse button is released.
     * \param event The mouse event containing release details.
     */
    void mouseReleased(QMouseEvent *event);

    /*!
     * \brief Emitted when the widget is resized.
     */
    void resized();

    /*!
     * \brief Emitted when the list of available video sources changes.
     * \param source The updated list of video source descriptions.
     */
    void videoSourcesChanged(QList<std::string> srcs);

    /*!
     * \brief Emitted when the list of available video formats for the current source changes.
     * \param formats The updated list of video format descriptions.
     */
    void videoFormatsChanged(QList<std::string> fmts); 
public slots:

    /*!
     * \brief Updates the displayed video frame.
     *
     * Retrieves the latest frame from the active video provider, applies middleware processing if any,
     * and updates the graphics scene with the new frame.
     */
    void updateFrame();

    /*!
     * \brief Changes the active video source.
     * \param src The description of the new video source.
     *
     * Updates the active provider and emits videoFormatsChanged with the new source's available formats.
     */
    void changeVideoSrc(const QString& src);

    /*!
     * \brief Changes the video format for the active source.
     * \param idx The index of the format in the list of available formats.
     */
    void changeVideofmt(int idx);

    /*!
     * \brief Increases the zoom level of the video.
     *
     * Increments the zoom factor by ZOOM_FACTOR, up to a maximum of 1.0, and updates the view.
     */
    void incZoom();

    /*!
     * \brief Decreases the zoom level of the video.
     *
     * Decrements the zoom factor by ZOOM_FACTOR, down to a minimum of 0.05, and updates the view.
     */
    void decZoom();

    /*!
     * \brief Fits the video frame to the widget's view.
     *
     * Resets the zoom factor to 1.0 and adjusts the view to fit the current frame while maintaining aspect ratio.
     */
    void fit();

    /*!
     * \brief Enables or disables edge detection middleware.
     * \param use If true, adds an edge detector middleware; if false, removes it.
     */
    void useEdgeDetector(bool use);

    /*!
     * \brief Adds an RTSP video source.
     * \param url The RTSP URL of the video source.
     *
     * Creates a new TRTCPFrameProvider, sets the URL, and starts the provider.
     */
    void addRTCPsource(QString url);
protected:
    /*!
     * \brief Handles mouse press events.
     * \param event The mouse event.
     *
     * Emits the mousePressed signal.
     */
    void mousePressEvent(QMouseEvent *event) override { emit mousePressed(event); }

    /*!
     * \brief Handles mouse move events.
     * \param event The mouse event.
     *
     * Emits the mouseMoved signal.
     */
    void mouseMoveEvent(QMouseEvent *event) override { emit mouseMoved(event); }

    /*!
     * \brief Handles mouse release events.
     * \param event The mouse event.
     *
     * Emits the mouseReleased signal.
     */
    void mouseReleaseEvent(QMouseEvent *event) override { emit mouseReleased(event); }

    /*!
     * \brief Handles resize events.
     * \param event The resize event.
     *
     * Calls the base class implementation and emits the resized signal.
     */
    void resizeEvent(QResizeEvent *event) override {
        QGraphicsView::resizeEvent(event);
        emit resized();
    }
private:
    std::unique_ptr<QGraphicsScene> scene_;                        ///< Graphics scene for displaying
    TSurfacePainter *painter_;                                     ///< Painter for drawing for measurement on the scene.
    QList<IFrameProvider* > fproviders_;                           ///< List of video frame providers.
    std::vector<std::unique_ptr<IFrameMiddleware> > fmiddlewares_; ///< List of middleware processors for frames.
    std::unique_ptr<QTimer> updateFrame_;                          ///< Timer for periodic frame updates.
    std::unique_ptr<QGraphicsPixmapItem > currentFrame_;           ///< Current video frame as a pixmap item.
    int currentActiveVideoProviderIdx_ = 0;                        ///< Current video source idx.
    std::string currentActiveFormatSrc_{};                         ///< Current video avaliable formats description.
    QList<std::string> videosrcDesc_;                              ///< List of available video source descriptions.
    QList<std::string> videofmtDesc_;                              ///< List of available video format descriptions.
    IFrameProvider* usbDevs_;                                      ///< USB video device provider (default provider).
    IFrameProvider* rtcp_;                                         ///< RTSP video provider.
    double zoomFactor_ = 0.5;                                      ///< Current zoom factor.
    QImage currentFrameImg_;                                       ///< Current frame data as a QImage.

    /*!
     * \brief Updates the video size and scene properties based on the frame.
     * \param img The current frame image.
     *
     * Adjusts the scene rectangle, painter font size, and line width based on the image dimensions.
     */
    void updateVideoSize(const QImage &img);

    /*!
     * \brief Adds a middleware processor to the frame processing chain.
     * \param middleware The middleware to add.
     */
    void addMiddleware(IFrameMiddleware *middleware);

    /*!
     * \brief Removes all middleware processors of a specific type.
     * \tparam T The type of middleware to remove.
     */
    template<typename T>
    void removeMiddlewareByType();

    /*!
     * \brief Removes all edge detector middleware processors.
     */
    void removeAllEdgeDetectors();

    /*!
     * \brief Checks if a middleware processor of a specific type exists.
     * \tparam T The type of middleware to check for.
     * \return True if the middleware exists, false otherwise.
     */
    template<typename T>
    bool findMiddlewareByType();
};

#endif // TVIDEOWDG_H
