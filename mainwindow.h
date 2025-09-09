/** @file
 *
 * TVideoWdg - SimpleMeasurementByVideoTool
 * By Ilia.belou <ilia.belou@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QActionGroup>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/*!
 * \class MainWindow
 * \brief Mainwindow with example usage of TVideoWdg.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    void initializeToolBar();
};
#endif // MAINWINDOW_H
