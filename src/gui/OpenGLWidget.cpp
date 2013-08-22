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

#include "OpenGLWidget.hpp"
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <cmath>
#include <QtGui>
#include "SimulationUtilities.hpp"

using namespace std;

namespace Feldrand {

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::Rgba |
						  QGL::DoubleBuffer |
						  QGL::SampleBuffers |
						  QGL::DepthBuffer), parent),
      draw_streamlines(),
      draw_arrows(),
      draw_lic(600, 150),
      drawing_routine(&draw_arrows)
{
    circle_mask = createCircleMask(20);
    onIdle();
}

OpenGLWidget::~OpenGLWidget()
{
}

QSize OpenGLWidget::minimumSizeHint() const
{
    return QSize(28, 8); // TODO
}

QSize OpenGLWidget::sizeHint() const
{
    return QSize(1120, 320); // TODO
}

void
OpenGLWidget::setSimulation(shared_ptr<Simulation> sim)
{
    lock_guard<mutex> lock(renderMutex);
    this->sim = sim;
}

void
OpenGLWidget::setVisualisation(vis_t vis) {
    lock_guard<mutex> lock(renderMutex);
    switch(vis) {
    case vis_t::PLAIN:
        drawing_routine = &draw_plain;
        break;
    case vis_t::STREAMLINES:
        drawing_routine = &draw_streamlines;
        break;
    case vis_t::ARROWS:
        drawing_routine = &draw_arrows;
        break;
    case vis_t::LIC:
        drawing_routine = &draw_lic;
        break;
    default:
        // leave it as it is
        break;
    }
}

void
OpenGLWidget::setColor(color_t color)
{
    lock_guard<mutex> lock(renderMutex);
    switch(color) {
    case color_t::MONO:
        draw_plain.useColorMono();
        draw_lic.useColorMono();
        draw_arrows.useColorMono();
        draw_streamlines.useColorMono();
        break;
    case color_t::DENSITY:
        draw_plain.useColorScalarField();
        draw_lic.useColorScalarField();
        draw_arrows.useColorScalarField();
        draw_streamlines.useColorScalarField();
        break;
    case color_t::VELOCITY:
        draw_plain.useColorVectorField();
        draw_lic.useColorVectorField();
        draw_arrows.useColorVectorField();
        draw_streamlines.useColorVectorField();
        break;
    default:
        // leave it as it is
        break;
    }
}

void
OpenGLWidget::initializeGL()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LINE_SMOOTH);
   
    glEnable(GL_BLEND);
    glLineWidth(1.0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void
OpenGLWidget::paintGL() {
    draw();
}

void
OpenGLWidget::resizeGL(int width, int height)
{
    lock_guard<mutex> lock(renderMutex);
    if(!sim) return;
    float swidth = sim->get<double>(Simulation::Data::width);
    float sheight = sim->get<double>(Simulation::Data::height);
    float wscale = (float)width / swidth;
    float hscale = (float)height / sheight;
    int w, h;
    int w_offset, h_offset;
    if(wscale < hscale) {
        w = width; h = sheight * wscale;
        h_offset = (height - h) / 2;
        w_offset = 0;
    } else {
        h = height; w = swidth * hscale;
        w_offset = (width - w) / 2;
        h_offset = 0;
    }
    glViewport((GLsizei)w_offset, (GLsizei)h_offset,
               (GLsizei)w, (GLsizei)h);
    horizontal_extent = w;
    vertical_extent   = h;
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    mouseMoveEvent(event);
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    lock_guard<mutex> lock(renderMutex);
    if(!sim) return;
    /* widget coordinates */
    int ww = this->width();
    int wh = this->height();
    int wx = event->x();
    int wy = event->y();

    /* drawing area coordinates */
    int dw = horizontal_extent;
    int dh = vertical_extent;
    int dx = wx - (ww - dw) / 2;
    int dy = wy - (wh - dh) / 2;

    /* simulation coordinates */
    int sx = (dx * sim->get<size_t>(Simulation::Data::gridWidth)) / dw;
    int sy = (dy * sim->get<size_t>(Simulation::Data::gridHeight)) / dh;

    if (event->buttons() & Qt::LeftButton) {
        Simulation::draw_data data = {sx, sy, circle_mask,
                                      cell_t::OBSTACLE};
        sim->action<Simulation::draw_data&>(Simulation::Action::draw, data);
    } else if (event->buttons() & Qt::RightButton) {
        Simulation::draw_data data = {sx, sy, circle_mask,
                                      cell_t::FLUID};
        sim->action<Simulation::draw_data&>(Simulation::Action::draw, data);
    }

    lastPos = event->pos();
}

bool
OpenGLWidget::draw() {
    lock_guard<mutex> lock(renderMutex);
    if(!sim) return false;

    sim->beginMultiple();
    auto v = sim->get<Grid<Vec2D<float>>*>(Simulation::Data::velocity_grid);
    auto d = sim->get<Grid<float>*>(Simulation::Data::density_grid);
    auto t = sim->get<Grid<cell_t>*>(Simulation::Data::type_grid);
    sim->endMultiple();

    vel_ptr = shared_ptr<const Grid<Vec2D<float>>>((Grid<Vec2D<float>>*)v);
    dens_ptr = shared_ptr<const Grid<float>>((Grid<float>*)d);
    return redraw();
}

bool
OpenGLWidget::redraw() {
    glColor3f(1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION) ;
    glLoadIdentity();

    // the drawing routines work on a unity square
    // with (0.0, 0.0) being the upper left corner
    glOrtho(0.0, 1.0, 1.0, 0.0, -5, 5);

    (*drawing_routine)(*vel_ptr, *dens_ptr);

	
    return true;
}

void
OpenGLWidget::onIdle() {
    updateGL();
    // every 16ms -> roughly 60fps
    QTimer::singleShot( 33, this, SLOT(onIdle()));
}
}
