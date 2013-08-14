/* Copyright (C) 2013  Marco Heisig, Dominik Ernst

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


	Grid<Vec2D<float>> smoothed_field = vector_field;
	//smooth
	for( size_t y = 0; y < vector_field.y(); y++) {
		for( size_t x = 0; x < vector_field.x(); x++) {
			if( x == 0 || x == vector_field.x()-1 ||
				y == 0 || y == vector_field.y()-1 ) {
				smoothed_field(x, y) = vector_field(x, y);
			} else {
				smoothed_field(x,y) = 
					( vector_field(x-1, y-1) * 2.0f +
					  vector_field(x-1, y  ) * 2.0f +
					  vector_field(x-1, y+1) * 2.0f +
					  vector_field(x  , y-1) * 2.0f +
					  vector_field(x  , y  ) * 2.0f +
					  vector_field(x  , y+1) * 2.0f +
					  vector_field(x+1, y-1) * 2.0f +
					  vector_field(x+1, y  ) * 2.0f +
					  vector_field(x+1, y+1) * 2.0f) * (1.0f/18.0f);

			}
		}
	}

   
	vector< Vec2D<float> > seeds;

    for(size_t i = 0; i < 70; ++i) {
        // generate qusirandom point, more uniform distribution
        size_t ix = i*1023%vector_field.x();
        size_t iy = i*222%vector_field.y();
        Vec2D<float> point((float)ix / (float)vector_field.x(),
                     (float)iy / (float)vector_field.y());
		seeds.push_back(point);

	}
	while(! seeds.empty() ) {
		Vec2D<float> point = seeds.back();
		seeds.pop_back();
		drawStreamline(point,  1.0, smoothed_field, scalar_field, seeds);
        drawStreamline(point, -1.0, smoothed_field, scalar_field, seeds);

    }
}

namespace {

	/* Calculates the nearest, positive intersection with the grid. Expects and returns coordinates in grid space. */
	Vec2D<float> step( Vec2D<float> origin, Vec2D<float> v) {
		
		
		// Determine nearest grid line
		float tx1 = ( floor(origin.x) - origin.x ) / v.x;
		float tx2 = ( ceil (origin.x) - origin.x ) / v.x;
		float ty1 = ( floor(origin.y) - origin.y ) / v.y;
		float ty2 = ( ceil (origin.y) - origin.y ) / v.y;
		
		

		float tx = (tx1 > tx2) ? tx1 : tx2;
		float ty = (ty1 > ty2) ? ty1 : ty2;

		float t = (tx < ty) ? tx : ty;

		return origin + v*t;	
	}

}


void
DrawStreamlinesImplementation::
drawStreamline(Vec2D<float> point,
               float dir,
               const Grid<Vec2D<float>>& vector_field,
               const Grid<float>& scalar_field,
			   vector<Vec2D<float>>& seeds) {

	
	Vec2D<float> gridpoint { point.x * (vector_field.x()-1),
			                 point.y * (vector_field.y()-1) };


    glBegin(GL_LINE_STRIP);
    glVertex3f(point.x, point.y, 0.0f);
    for(size_t i = 0; i < 10000; i++) {
 
	
		Vec2D<float> v1 = interpolate( vector_field,
						  { gridpoint.x / (vector_field.x()-1), 
						    gridpoint.y / (vector_field.y()-1) } );


		v1 *= dir;
		v1.y *= -1; // Right hand vs left hand coordinate systems?
		

		// Calculate the predictor point and get the direction at that point.
		Vec2D<float> predictor = step( gridpoint, v1 );
		Vec2D<float> v2 = interpolate( vector_field,
									   { predictor.x / (vector_field.x()-1),
										 predictor.y / (vector_field.y()-1) } );
	
		
		v2 *= dir;
		v2.y *= -1; // Coordinate system correction
		

		v1 = (v1 + v2)/2.0;
		if( v1.x*v1.x+v1.y*v1.y < 0.00001) break;
		
		gridpoint = step( gridpoint, v1);

		if( gridpoint.x < 3 || gridpoint.x > vector_field.x()-3 || 
			gridpoint.y < 3 || gridpoint.y > vector_field.y()-3 ) break;
		
		//Drawing space coordinates
		Vec2D<float> point { gridpoint.x / (vector_field.x()-1), 
				             gridpoint.y / (vector_field.y()-1) };
	
		if( i % 1 == 0) {	
			QColor color = getColorAtPoint(vector_field, scalar_field, point);
			glColor4f(color.redF(),
					  color.greenF(),
					  color.blueF(),
					  1.0 );
			glVertex3f(point.x, point.y, 0.0f);
		}

		

		if( i%5 == 0) {
			for( auto  it = seeds.begin();
				 it != seeds.end();) {
				if( ((it->x-point.x)*(it->x-point.x)+
					 (it->y-point.y)*(it->y-point.y)) < 0.0005 ) {
					it = seeds.erase(it);
				} else {
					it++;
				}
			}
		}
		
    }
    glEnd();
}
}
