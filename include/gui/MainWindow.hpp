/* Copyright (C) 2013  Marco Heisig

This file is part of Feldrand.

Feldrand is free software: you can redistribute it and/or modify it under the
terms of the GNU Affero General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more
details.

You should have received a copy of the GNU Affero General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef FELDRAND__MAIN_WINDOW_HPP
#define FELDRAND__MAIN_WINDOW_HPP

#include <memory>
#include <QMainWindow>

//#include "OpenGLWidget.hpp"
#include "Simulation.hpp"

QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
class QLabel;
class QMenu;
QT_END_NAMESPACE

namespace Feldrand {

class OpenGLWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void newFile();
    void open();
    void save();

    void clear();
    void pause();
    void resume();

    void colorMono();
    void colorVelocity();
    void colorDensity();

    void visPlain();
    void visStreamlines();
    void visArrows();
    void visLic();
	void screenshot();
    void fullscreen();

    void about();
private:
    void createActions();
    void createMenus();

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *colorMenu;
    QMenu *visMenu;
    QMenu *helpMenu;

    QActionGroup *alignmentGroup;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *exitAct;

    QAction *colorMonoAct;
    QAction *colorVelocityAct;
    QAction *colorDensityAct;

    QAction *visPlainAct;
    QAction *visStreamlinesAct;
    QAction *visArrowsAct;
    QAction *visLicAct;
	QAction *fullscreenAct;
    QAction *screenshotAct;


    QAction *clearAct;
    QAction *pauseAct;
    QAction *resumeAct;

    QAction *aboutAct;
    OpenGLWidget *openGLWidget;
private:
    std::shared_ptr<Simulation> sim;
};
}
#endif // FELDRAND__MAIN_WINDOW_HPP
