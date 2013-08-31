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

#ifndef FELDRAND__OPENGL_WIDGET_HPP
#define FELDRAND__OPENGL_WIDGET_HPP

#include <GL/glew.h> //Has to be included bedore QGLWidget
#include <memory>
#include <mutex>
#include <QGLWidget>
#include "Simulation.hpp"
#include "DrawPlain.hpp"
#include "DrawLIC.hpp"
#include "DrawStreamlines.hpp"
#include "DrawArrows.hpp"

namespace Feldrand {

enum struct color_t {
    MONO,
    DENSITY,
    VELOCITY
    };

enum struct vis_t {
    PLAIN,
    STREAMLINES,
    ARROWS,
    LIC
};

class OpenGLWidget : public QGLWidget
{
    Q_OBJECT

public:
    OpenGLWidget(QWidget *parent = 0);
    ~OpenGLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

public:
    void setSimulation(std::shared_ptr<Simulation> sim);
    void setVisualisation(vis_t vis);
    void setColor(color_t color);
	void takeScreenshot();

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);


protected:
    /* get simulation values and visualize them */
    bool draw();

    /* visualize without fetching new values*/
    bool redraw();

private slots:
    void onIdle();

protected:
    std::mutex renderMutex;
    std::shared_ptr<Simulation> sim;
    std::shared_ptr<const Grid<mask_t>> circle_mask;
    std::shared_ptr<const Grid<Vec2D<float>>> vel_ptr;
    std::shared_ptr<const Grid<float>> dens_ptr;
    QPoint lastPos;
    /* widget surface occupied by the simulation */
    int vertical_extent;
    int horizontal_extent;
    DrawPlain draw_plain;
    DrawStreamlines draw_streamlines;
    DrawArrows draw_arrows;
    DrawLIC draw_lic;
    DrawingRoutine* drawing_routine;
};
}
#endif // FELDRAND__OPENGL_WIDGET_HPP
