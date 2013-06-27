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

#include "DrawStreamlinesImplementation.hpp"
#include <cstdlib>
#include <GL/gl.h>
#include <GL/glu.h>

using namespace std;

namespace Feldrand {

void
DrawStreamlinesImplementation::
operator()(const Grid<Vec2D<float>>& vector_field,
           const Grid<float>& scalar_field) {
    calibrateColor(vector_field, scalar_field);
    // use fixed seed as we want the same starting points each run.
    srand(42);
    for(size_t i = 0; i < streamline_amount; ++i) {
        // generate random point
        size_t ix = (rand() % vector_field.x());
        size_t iy = (rand() % vector_field.y());
        Vec2D<float> point((float)ix / (float)vector_field.x(),
                     (float)iy / (float)vector_field.y());
        drawStreamline(point,  1.0, vector_field, scalar_field);
        drawStreamline(point, -1.0, vector_field, scalar_field);
    }
}

void
DrawStreamlinesImplementation::
drawStreamline(Vec2D<float> point,
               float dir,
               const Grid<Vec2D<float>>& vector_field,
               const Grid<float>& scalar_field) {
    float cell_size =  1.0 / vector_field.x();
    float scaling = cell_size / steps_per_cell;
    glBegin(GL_LINE_STRIP);
    glVertex3f(point.x, point.y, 0.0f);
    for(size_t i = 0; i < 1.0 / scaling; i++) {
        // using Heun's method which is second order accurate
        Vec2D<float> euler = point
            + interpolate(vector_field, point).normalize() * dir * scaling;
        point += euler
            + interpolate(vector_field, euler).normalize() * dir * scaling;
        point /= 2.0;
        if(interpolate(vector_field, point).abs() == 0.0) break;

        QColor color = getColorAtPoint(vector_field, scalar_field, point);
        glColor3f(color.redF(), color.greenF(), color.blueF());
        glVertex3f(point.x, point.y, 0.0f);
    }
    glEnd();
}
}
