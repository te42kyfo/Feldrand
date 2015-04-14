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

#include "DrawPlainImplementation.hpp"
#include <cstdlib>
#include <cmath>
#include <GL/gl.h>
#include <GL/glu.h>
#include <vector>
#include <iostream>
using namespace std;

namespace Feldrand {

DrawPlainImplementation::DrawPlainImplementation() {
}

DrawPlainImplementation::~DrawPlainImplementation() {
}

void DrawPlainImplementation::operator()(const Grid<Vec2D<float>>& vector_field,
                                         const Grid<float>& scalar_field) {
    size_t width = vector_field.x() - 1;
    size_t height = vector_field.y() - 1;
    float iwidth = 1.0f / (float)width;
    float iheight = 1.0f / (float)height;
    vector<float> texture(width * height * 3);
    calibrateColor(vector_field, scalar_field);
    for(size_t iy = 0; iy < height; ++iy) {
        for(size_t ix = 0; ix < width; ++ix) {
            size_t index = 3 * ((iy * width) + ix);
            float fx = (float)ix * iwidth;
            float fy = (float)iy * iheight;

            QColor color = getColorAtPoint(vector_field, scalar_field, {fx, fy});

            texture[index]     = color.redF();
            texture[index + 1] = color.greenF();
            texture[index + 2] = color.blueF();
        }
    }

    GLuint t;
    glGenTextures( 1, &t);
    glEnable(GL_TEXTURE_2D);
    glBindTexture( GL_TEXTURE_2D, t);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,
                  width, height,
                  0, GL_RGB, GL_FLOAT, texture.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0, 0.0, 0.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, 0.0, 0.0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0, 1.0, 0.0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0, 1.0, 0.0);
    glEnd();

    glDeleteTextures(1, &t);
}
}
