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

#ifndef FELDRAND__DRAWING_ROUTINE_HPP
#define FELDRAND__DRAWING_ROUTINE_HPP

#include <QColor>
#include "Grid.hpp"
#include "Vec2D.hpp"

namespace Feldrand {

/* This is the interface for all avilable visualisations.
 * Each DrawingRoutine is called with a reference to a vector and a scalar
 * field (both rectangular and of equal size) and visualizes these on a
 * (0,1)x(0,1) Rectangle via OpenGL.
 */
class DrawingRoutine {
protected:
    DrawingRoutine();
public:
    virtual ~DrawingRoutine();

    virtual void operator()(const Grid<Vec2D<float>>& vector_field,
                            const Grid<float>&  scalar_field) = 0;

    /* Make this drawing routine draw with the color given via
     * setMonoColor().
     */
    void useColorMono();

    /* Make this drawing routine draw with color depending on the given
     * scalar field. Given a certain toleranc percentage via setTolerance(),
     * a minimum color from setMinColor() and a maximum color from
     * setMaxColor(), each point will be colored the following way:
     * - The <tolerance> percent highest values in the grid get the maximum
     *   color.
     * - The <tolerance> percent lowest values in the grid get the
     *   minimum color.
     * - The rest of the grid entrys get a color between the minimum and the
     *   maximum color depending on their value.
     */
    void useColorScalarField();

    /* Make this drawing routine draw with color depending on the given
     * vector field. The procedure and parameters are the same as in
     * useColorScalarField() except instead of the scalar value at each
     * point it uses the euclidean norm of each vector.
     */
    void useColorVectorField();

    /* The color used after a call to useColorMono()
     */
    void setMonoColor(QColor color);
    QColor getMonoColor() const;

    /* The color used for the highest values after a call to
     * useColorVectorField() of useColorScalarField()
     */
    void setMaxColor (QColor color);
    QColor getMaxColor() const;

    /* The color used for the lowest values after a call to
     * useColorVectorField() of useColorScalarField()
     */
    void setMinColor (QColor color);
    QColor getMinColor() const;

    /* set the percentage of the maximal and minimal values of a
     * field that shall be drawn with equal color. The remaining
     * points are then colored linearly between the lowest value of the
     * maximal points and the highest value of the minimal points.
     * The purpose of the tolerance is to avoid that some few extreme
     * values leave the rest of the field with almost homogeneous color.
     * The tolerance should be between 0.0f and 0.5f
     */
    void setTolerance(float tolerance);
    float getTolerance() const;

public:
    class DrawingRoutineImplementation;
protected:
    DrawingRoutineImplementation* impl;
};

}
#endif // FELDRAND__DRAWING_ROUTINE_HPP
