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
   

	// Funky single pass mean/standard deviation
	float M = 0; 
	float Q = 0;
	size_t k = 1;
	// copy all values to a buffer and sort them there
    if(use_color_mono) return;
	
	float average = 0;

	for(size_t iy = 0; iy < scalar_field.y(); iy+=2) {
		for(size_t ix = 0; ix < scalar_field.x(); ix++) {
				
			float x;
			if( use_color_vec_abs )
				x  = vector_field(ix, iy).sqlength();
			else 
				x = scalar_field(ix, iy);	
			
			average += x;

			if( k == 1) {
				M = x;
				Q = 0;
			} else {
				Q += (k-1) * (x-M)*(x-M) / k; 					
				M += (x - M) / k;
			}
			k++;
		}
	}

	float varSquared = Q/k;
	float a = 1;
	float b = 2*M;
	float c = -varSquared;

	float var = (-b + sqrt( b*b - 4*a*c) ) / 2 / a;
	float sd = sqrt(var);

	//std::cout << "\n" << sqrt(M) << " " << sd << "\n";

	max_value = sqrt(M) + 1.7*sd;
	min_value = sqrt(M) - 2*sd;


    if(max_value - min_value <= 0.00001) min_value = max_value - 0.0001;
}

}
