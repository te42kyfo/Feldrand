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

#ifndef FELDRAND__DRAWING_ROUTINE_IMPLMENTATION_HPP
#define FELDRAND__DRAWING_ROUTINE_IMPLMENTATION_HPP

#include <memory>
#include "DrawingRoutine.hpp"
#include "Grid.hpp"
#include "Vec2D.hpp"

namespace Feldrand {

class DrawingRoutine::DrawingRoutineImplementation {
protected:
    DrawingRoutineImplementation();

public:
    virtual ~DrawingRoutineImplementation() {}

    virtual void operator()(const Grid<Vec2D<float>>& velocity,
                            const Grid<float>& density) = 0;
    void useColorMono();

    void useColorScalarField();

    void useColorVectorField();

    void setMonoColor(QColor color);
    QColor getMonoColor() const;

    void setMaxColor (QColor color);
    QColor getMaxColor() const;

    void setMinColor (QColor color);
    QColor getMinColor() const;

    void setTolerance(float tolerance);
    float getTolerance() const;

protected:
    /* calculate the color palette according to the current min
     * and max values with respect to tolerance
     */
    void calibrateColor(const Grid<Vec2D<float>> vector_field,
                        const Grid<float> scalar_field);

private:
    bool use_color_mono;
    bool use_color_scalar;
    bool use_color_vec_abs;

    QColor mono_color;
    QColor max_color;
    QColor min_color;
    float tolerance;

    float min_value;
    float max_value;

protected:
    /* interpolate on a point p in (0.0, 1.0)x(0.0, 1.0)
     */
    template<typename T>
    inline
    T interpolate(const Grid<T>& grid, Vec2D<float> point) const {
        float width  = grid.x() - 1;
        float height = grid.y() - 1;
        size_t ix = static_cast<size_t>(point.x * (float)width);
        size_t iy = static_cast<size_t>(point.y * (float)height);
        if(ix > width - 2 || iy > height - 2 ||
           ix < 0          || iy < 0) {
            return T();
        }
        float hx = 1.0 / (float)width;
        float hy = 1.0 / (float)height;
        Vec2D<float> upper_left  = {ix * hx, iy * hy};
        point -= upper_left;
        float weight_x = point.x / hx;
        float weight_y = point.y / hy;

        // bilinear interpolation
        T mean = T();
        mean += grid(ix, iy)         * (1 - weight_x) * (1 - weight_y);
        mean += grid(ix + 1, iy)     *      weight_x  * (1 - weight_y);
        mean += grid(ix, iy + 1)     * (1 - weight_x) *      weight_y ;
        mean += grid(ix + 1, iy + 1) *      weight_x  *      weight_y ;
        return mean;
    }

    template<typename T>
    inline
    T& grid_ref(Grid<T>& grid, Vec2D<float> point) const {
        float width  = grid.x() - 1;
        float height = grid.y() - 1;
        size_t ix = static_cast<size_t>(point.x * (float)width);
        size_t iy = static_cast<size_t>(point.y * (float)height);
        return grid(ix, iy);
    }

    template<typename T>
    inline
    const T& grid_ref(const Grid<T>& grid, Vec2D<float> point) const {
        float width  = grid.x() - 1;
        float height = grid.y() - 1;
        size_t ix = static_cast<size_t>(point.x * (float)width);
        size_t iy = static_cast<size_t>(point.y * (float)height);
        return grid(ix, iy);
    }

    inline
    QColor getColorAtPoint(const Grid<Vec2D<float>>& vector_field,
                             const Grid<float>& scalar_field,
                             const Vec2D<float> point) {
        float value;
        if(use_color_mono) return mono_color;
        if(use_color_vec_abs)
            value = interpolate(vector_field, point).abs();
        else // use_color_scalar
            value = interpolate(scalar_field, point);

        if(value > max_value) value = max_value;
        if(value < min_value) value = min_value;

        float d = max_value - min_value;
        float min = min_value;
        float s = (value - min) / d;

        float hmin = min_color.hueF();
        float hmax = max_color.hueF();
        float h = hmin + s * (hmax - hmin);
        return QColor::fromHsvF( h, 1.0, 1.0);
    }
};
}
#endif // FELDRAND__DRAWING_ROUTINE_IMPLMENTATION_HPP
