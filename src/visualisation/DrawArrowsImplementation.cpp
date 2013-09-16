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

#include "DrawArrowsImplementation.hpp"
#include <cmath>
#include <GL/gl.h>
#include <GL/glu.h>
using namespace std;

namespace Feldrand {

DrawArrowsImplementation::
DrawArrowsImplementation() {}

DrawArrowsImplementation::
~DrawArrowsImplementation() {}

void
DrawArrowsImplementation::
operator()(const Grid<Vec2D<float>>& vector_field,
           const Grid<float>& scalar_field) {
    calibrateColor(vector_field, scalar_field);
    const size_t cells_per_dim = 8;
    const size_t cells_per_arrow = cells_per_dim * cells_per_dim;
    for(size_t iy = 0; iy < vector_field.y() - cells_per_dim; iy += cells_per_dim) {
        for(size_t ix = 0; ix < vector_field.x() - cells_per_dim; ix += cells_per_dim) {
            Vec2D<float> mean = Vec2D<float>();
            // gather mean direction over all relevant cells
            for(size_t dy = 0; dy < cells_per_dim; dy += 1) {
                for(size_t dx = 0; dx < cells_per_dim; dx += 1) {
                    mean += vector_field(ix + dx, iy + dy);
                }
            }
            mean /= (float) cells_per_arrow;
            if(mean.abs() < 0.0001) continue;
            glPushMatrix();
            Vec2D<float> point = {(float)ix / (float)vector_field.x(),
                                  (float)iy / (float)vector_field.y()};
            glTranslatef(point.x, point.y, 0.0f);
            glScalef(1.0 / (float)vector_field.x() * cells_per_dim,
                     1.0 / (float)vector_field.y() * cells_per_dim, 1.0f);
            glTranslatef(0.5f, 0.5f, 0.0f);
            glRotatef(atan2(mean.y,mean.x) / (2.0f * M_PI) * 360.0f,
                      0.0f, 0.0f, 1.0f);
            QColor color = getColorAtPoint(vector_field, scalar_field, point);
            glColor3f(color.redF(), color.greenF(), color.blueF());
            drawArrow();
            glPopMatrix();
        }
    }
}

void DrawArrowsImplementation::drawArrow() {
    glBegin( GL_TRIANGLES );
    glVertex3f( 0.5,  0.0, 0.0f);
    glVertex3f( 0.0,  0.2, 0.0f);
    glVertex3f( 0.0, -0.2, 0.0f);

    glVertex3f(-0.5, -0.05, 0.0f);
    glVertex3f( 0.0, -0.05, 0.0f);
    glVertex3f( 0.0,  0.05, 0.0f);

    glVertex3f(-0.5, -0.05, 0.0f);
    glVertex3f(-0.5,  0.05, 0.0f);
    glVertex3f( 0.0,  0.05, 0.0f);
    glEnd();
}
}
