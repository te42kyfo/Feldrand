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

#include "DrawLICImplementation.hpp"
#include <cstdlib>
#include <cmath>
#include <GL/gl.h>
#include <GL/glu.h>
#include <vector>
#include <iostream>
using namespace std;

namespace Feldrand {

DrawLICImplementation::DrawLICImplementation(size_t width, size_t height)
    : random_texture(width, height),
      texture(width, height),
      marked(width, height) {
    for(size_t iy = 0; iy < height; ++iy) {
        for(size_t ix = 0; ix < width; ++ix) {
            random_texture(ix, iy) = rand() % 2;
        }
    }
}

DrawLICImplementation::~DrawLICImplementation() {
}

void DrawLICImplementation::operator()(const Grid<Vec2D<float>>& vector_field,
                         const Grid<float>& scalar_field) {
    refreshGrids();
    // a good integration range is a tenth of the domain
    size_t cells = texture.x() * texture.y();
    size_t L = sqrt(cells) / 10;
    size_t range = 2 * L + 1;
    float k = 1.0 / (float)range;
    float cell_size = 1.0 / texture.x();
    float step_size = cell_size * 0.5;
    // generate random point
    srand(42);
    for(size_t i = 0; i < texture.x() * texture.y() / 2; ++i) {
        size_t ix = (rand() % texture.x());
        size_t iy = (rand() % texture.y());
        if(marked(ix, iy) < 2) {
            Vec2D<float> point((float)ix / (float)texture.x(),
                         (float)iy / (float)texture.y());
            integrateStreamline(point, step_size, range, k, vector_field, scalar_field);
        }
    }
    for(size_t iy = 0; iy < texture.y(); ++iy) {
        for(size_t ix = 0; ix < texture.x(); ++ix) {
            if(marked(ix, iy) < 2) {
                Vec2D<float> point((float)ix / (float)texture.x(),
                             (float)iy / (float)texture.y());
            integrateStreamline(point, step_size, range, k, vector_field, scalar_field);
            }
        }
    }
    // normalize textures
    for(size_t iy = 0; iy < texture.y(); ++iy) {
        for(size_t ix = 0; ix < texture.x(); ++ix) {
            texture(ix, iy) /= marked(ix, iy);
        }
    }

    GLuint t;
    glGenTextures( 1, &t);
    glBindTexture( GL_TEXTURE_2D, t);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,
                  texture.x(), texture.y(),
                  0, GL_LUMINANCE, GL_FLOAT, texture.data());
    glEnable(GL_TEXTURE_2D);
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

void DrawLICImplementation::integrateStreamline(Vec2D<float> point, float step_size,
                                  size_t range, float k,
                                  const Grid<Vec2D<float>>& vector_field,
                                  const Grid<float>& scalar_field) {
    vector<float> ring(range, 0.5 * k);
    size_t ring_pos = 0;
    size_t tolerance = 5;
    Vec2D<float> up = point;
    for(size_t i = 0; i < range / 2; ++i) {
        Vec2D<float> euler = up
            + interpolate(vector_field, up).normalize() * step_size;
        up += euler
            + interpolate(vector_field, euler).normalize() * step_size;
        up /= 2.0;
        if(up.x <= 0.0 || up.x >= 1.0 || up.y <= 0.0 || up.y >= 1.0) break;
        if(grid_ref(marked, up) >= tolerance) break;
        ring[(ring_pos + i) % range] = k * interpolate(random_texture, up);
    }
    Vec2D<float> down = point;
    for(size_t i = 0; i < range / 2; ++i) {
        Vec2D<float> euler = down
            + interpolate(vector_field, down).normalize() * step_size * -1.0;
        down += euler
            + interpolate(vector_field, euler).normalize() * step_size * -1.0;
        down /= 2.0;
        if(down.x < 0.0 || down.x >= 1.0 || down.y < 0.0 || down.y >= 1.0) break;
        if(grid_ref(marked, down) >= tolerance) break;
        ring[(ring_pos + range - i) % range] = k * interpolate(random_texture, down);
    }

    // determine the first value
    float val = 0.0f;
    for(float& f : ring) {
        val += f;
    }
    grid_ref(texture, point) += val;
    grid_ref(marked,  point) += 1;

    // now the rest of the streamline
    vector<float> up_ring(ring);
    size_t up_pos = ring_pos;
    for(size_t i = 0;; ++i) {
        Vec2D<float> euler = up
            + interpolate(vector_field, up).normalize() * step_size;
        up += euler
            + interpolate(vector_field, euler).normalize() * step_size;
        up /= 2.0;
        if(up.x <= 0.0 || up.x >= 1.0 || up.y <= 0.0 || up.y >= 1.0) break;
        if(grid_ref(marked, up) >= tolerance) break;
        float f = k * interpolate(random_texture, up);

        ++up_pos;
        val -= up_ring[(up_pos + range / 2) % range];
        up_ring[(up_pos + range / 2) % range] = f;
        val += f;
        grid_ref(texture, up) += val;
        grid_ref(marked,  up) += 1;
    }
    vector<float>& down_ring = ring;
    size_t down_pos = ring_pos;
    for(size_t i = 0;; ++i) {
        Vec2D<float> euler = down
            + interpolate(vector_field, down).normalize() * step_size * -1.0;
        down += euler
            + interpolate(vector_field, euler).normalize() * step_size * -1.0;
        down /= 2.0;
        if(down.x <= 0.0 || down.x >= 1.0 || down.y <= 0.0 || down.y >= 1.0) break;
        if(grid_ref(marked, down) >= tolerance) break;
        float f = k * interpolate(random_texture, down);

        ++down_pos;
        val -= down_ring[(down_pos + range / 2) % range];
        down_ring[(down_pos + range / 2) % range] = f;
        val += f;
        grid_ref(texture, down) += val;
        grid_ref(marked,  down) += 1;
    }
}

void DrawLICImplementation::refreshGrids() {
    for(size_t iy = 0; iy < texture.y(); ++iy) {
        for(size_t ix = 0; ix < texture.x(); ++ix) {
            texture(ix, iy) = 0.5f;
            marked(ix, iy) = 1;
        }
    }
}
}
