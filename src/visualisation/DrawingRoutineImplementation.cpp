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

#include "DrawingRoutineImplementation.hpp"
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

namespace Feldrand {

DrawingRoutine::DrawingRoutineImplementation::
DrawingRoutineImplementation() {
    tolerance = 0.01f;
    mono_color = {  124, 124, 124};
    min_color  = QColor::fromHsvF(0.66, 1.0, 1.0);
    max_color  = QColor::fromHsvF(0, 1.0, 1.0);
    use_color_mono = false;
    use_color_scalar = false;
    use_color_vec_abs = true;
}

void
DrawingRoutine::DrawingRoutineImplementation::
useColorMono(){
    use_color_mono = true;
    use_color_scalar = false;
    use_color_vec_abs = false;
}

void
DrawingRoutine::DrawingRoutineImplementation::
useColorScalarField(){
    use_color_mono = false;
    use_color_scalar = true;
    use_color_vec_abs = false;
}

void
DrawingRoutine::DrawingRoutineImplementation::
useColorVectorField(){
    use_color_mono = false;
    use_color_scalar = false;
    use_color_vec_abs = true;
}

void
DrawingRoutine::DrawingRoutineImplementation::
setMonoColor(QColor color){
    this->mono_color = color;
}

QColor
DrawingRoutine::DrawingRoutineImplementation::
getMonoColor() const {
    return mono_color;
}

void
DrawingRoutine::DrawingRoutineImplementation::
setMaxColor(QColor color){
    max_color = color;
}

QColor
DrawingRoutine::DrawingRoutineImplementation::
getMaxColor() const {
    return max_color;
}

void
DrawingRoutine::DrawingRoutineImplementation::
setMinColor(QColor color){
    this->min_color = color;
}

QColor
DrawingRoutine::DrawingRoutineImplementation::
getMinColor() const {
    return min_color;
}

void
DrawingRoutine::DrawingRoutineImplementation::
setTolerance(float tolerance){
    this->tolerance = tolerance;
}

float
DrawingRoutine::DrawingRoutineImplementation::
getTolerance() const {
    return tolerance;
}

void
DrawingRoutine::DrawingRoutineImplementation::
calibrateColor(const Grid<Vec2D<float>> vector_field,
               const Grid<float> scalar_field) {
    size_t grid_points = scalar_field.x() * scalar_field.y();
    size_t tol = grid_points * tolerance;
    std::vector<float> buf(grid_points);

    // copy all values to a buffer and sort them there
    if(use_color_mono) return;
    if(use_color_vec_abs) {
        for(size_t iy = 0; iy < scalar_field.y(); ++iy) {
            for(size_t ix = 0; ix < scalar_field.x(); ++ix) {
                buf[iy * scalar_field.x() + ix] = vector_field(ix, iy).abs();
            }
        }
    } else {
        for(size_t iy = 0; iy < scalar_field.y(); ++iy) {
            for(size_t ix = 0; ix < scalar_field.x(); ++ix) {
                buf[iy * scalar_field.x() + ix] = scalar_field(ix, iy);
            }
        }
    }
    std::sort(buf.begin(), buf.end(), std::greater<float>());

    max_value = buf[tol];
    min_value = buf[grid_points - tol - 1];
    // avoid later division by zero if interval is too small
    if(max_value - min_value <= 0.00001) min_value = max_value - 0.0001;
}

}
