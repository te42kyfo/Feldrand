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
   
	vector< Vec2D<float> > seeds;

    for(size_t i = 0; i < 5000; ++i) {
        // generate random point
        size_t ix = i*1023%vector_field.x();
        size_t iy = i*222%vector_field.y();
        Vec2D<float> point((float)ix / (float)vector_field.x(),
                     (float)iy / (float)vector_field.y());
		seeds.push_back(point);

	}
	while(! seeds.empty() ) {
		Vec2D<float> point = seeds.back();
		seeds.pop_back();
		drawStreamline(point,  1.0, vector_field, scalar_field, seeds);
        drawStreamline(point, -1.0, vector_field, scalar_field, seeds);

    }
}

void
DrawStreamlinesImplementation::
drawStreamline(Vec2D<float> point,
               float dir,
               const Grid<Vec2D<float>>& vector_field,
               const Grid<float>& scalar_field,
			   vector<Vec2D<float>>& seeds) {

	

   

	float xcell = point.x * (vector_field.x()-1);
    float ycell = point.y * (vector_field.y()-1);


	bool predictor = true;;
	Vec2D<float> v1;

    glBegin(GL_LINE_STRIP);
    glVertex3f(point.x, point.y, 0.0f);
    for(size_t i = 0; i < 3000; i++) {
  
		
		if( predictor) {
			v1 = interpolate(vector_field, point );
			if( v1.x*v1.x+v1.y*v1.y < 0.000001) break;
			v1 = v1.normalize();
			v1.y *= dir* -1;
			v1.x *= dir;

		}
		


		float tx1 = (floor(xcell) - xcell) / v1.x;
		float tx2 = (floor(xcell) - xcell+ ( (v1.x<0) ? -1 : 1)  ) / v1.x;
		float ty1 = (floor(ycell) - ycell) / v1.y;
		float ty2 = (floor(ycell) - ycell+ ( (v1.y<0) ? -1 : 1)  ) / v1.y;
		

		float tx = (tx1 > tx2) ? tx1 : tx2;
		float ty = (ty1 > ty2) ? ty1 : ty2;

		Vec2D<float> intermediate;
		int intermediate_xcell = xcell;
		int intermediate_ycell = ycell;

		if( tx < ty) {
			intermediate.x = point.x + tx*v1.x/(vector_field.x()-1);
			intermediate.y = point.y + tx*v1.y/(vector_field.y()-1);
			intermediate_xcell = xcell + tx*v1.x;
		} else {
			intermediate.x = point.x + ty*v1.x/(vector_field.x()-1);
			intermediate.y = point.y + ty*v1.y/(vector_field.y()-1);
			intermediate_ycell = ycell + ty*v1.y ;
		}

		
		if( predictor) {
			Vec2D<float> v2 = interpolate(vector_field, intermediate);
			
			v2 = v2.normalize();
			v2.y *= dir* -1;
			v2.x *= dir;
	
			v1 = (v1 + v2)/2.0;

			predictor = false;
		} else {
			point = intermediate;
			xcell = intermediate_xcell;
			ycell = intermediate_ycell;

			if( xcell < 10 || xcell > vector_field.x()-10 || 
				ycell < 10 || ycell > vector_field.y()-10 ) break;
			
			if( i % 1 == 0) {
				QColor color = getColorAtPoint(vector_field, scalar_field, point);
				glColor4f(color.redF(), color.greenF(), color.blueF(), 1-color.blueF()*color.redF()*2 );
				glVertex3f(point.x, point.y, 0.0f);
			}
			if( i%5 == 0) {
				for( auto  it = seeds.begin();
					 it != seeds.end();) {
					if( ((it->x-point.x)*(it->x-point.x)+
						 (it->y-point.y)*(it->y-point.y)) < 0.002 ) {
						it = seeds.erase(it);
					} else {
						it++;
					}
				}
			}
			predictor = true;
		}
    }
    glEnd();
}
}
