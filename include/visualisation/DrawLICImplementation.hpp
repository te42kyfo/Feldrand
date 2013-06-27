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

#ifndef FELDRAND__DRAW_LIC_IMPLEMENTATION_HPP
#define FELDRAND__DRAW_LIC_IMPLEMENTATION_HPP

#include "DrawingRoutineImplementation.hpp"

namespace Feldrand {

class DrawLICImplementation
    : public DrawingRoutine::DrawingRoutineImplementation {
public:
    DrawLICImplementation(size_t width, size_t height);

    ~DrawLICImplementation();

    virtual void operator()(const Grid<Vec2D<float>>& vector_field,
                            const Grid<float>& scalar_field);

protected:
    /* update all pixels along one streamline
     * point: the the starting point of the streamline
     * range: how many streamline points contribute to one texture pixel
     */
    void integrateStreamline(Vec2D<float> point, float step_size,
                             size_t range, float k,
                             const Grid<Vec2D<float>>& vector_field,
                             const Grid<float>& scalar_field);

    /* prepare the internal Grids for usage by the LIC algorithm. This means:
     * 1. set each entry of marked to zero.
     * 1. set each entry of texture to zero.
     */
    void refreshGrids();

protected:
    Grid<float> random_texture;
    Grid<float> texture;
    Grid<size_t> marked;
};
}
#endif // FELDRAND__DRAW_LIC_IMPLEMENTATION_HPP
