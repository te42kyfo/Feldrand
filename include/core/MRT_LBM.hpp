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

#ifndef FELDRAND__MRT_LBM_HPP
#define FELDRAND__MRT_LBM_HPP

#include "core/SimulationImplementation.hpp"

namespace Feldrand {

/* a cell consists of 9 entrys + the cell type */
struct Cell {
    float NW, N, NE;
    float W,  C,  E;
    float SW, S, SE;
    cell_t type;
};

std::ostream& operator<<(std::ostream& dest, const Cell& cell);
std::istream& operator>>(std::istream& src, Cell& cell);

const Cell fluid = {
    1.0f/36.0f, 1.0f/9.0f, 1.0f/36.0f,
    1.0f/9.0f,  4.0f/9.0f, 1.0f/9.0f,
    1.0f/36.0f, 1.0f/9.0f, 1.0f/36.0f,
    cell_t::FLUID
};

const Cell source = {
    1.6f/36.0f, 1.6f/9.0f, 1.6f/36.0f,
    1.6f/9.0f,  1.6f/9.0f, 1.6f/9.0f,
    1.6f/36.0f, 1.6f/9.0f, 1.6f/36.0f,
    cell_t::CONSTANT
};

const Cell drain = {
    0.55f/36.0f, 0.55f/9.0f, 0.55f/36.0f,
    0.55f/9.0f,   5.75f/9.0f, 0.55f/9.0f,
    0.55f/36.0f, 0.55f/9.0f, 0.55f/36.0f,
    cell_t::CONSTANT
};

const Cell obstacle = {
    1.0f/36.0f, 1.0f/9.0f, 1.0f/36.0f,
    1.0f/9.0f,  4.0f/9.0f, 1.0f/9.0f,
    1.0f/36.0f, 1.0f/9.0f, 1.0f/36.0f,
    cell_t::OBSTACLE
};

class MRT_LBM : public Simulation::SimulationImplementation {
public:
    MRT_LBM();
    MRT_LBM(double width, double height, size_t grid_width, size_t grid_height);
    MRT_LBM(MRT_LBM& other);
    virtual ~MRT_LBM();
protected:
    void one_iteration();
    void do_clear();
    void do_draw(int x, int y,
                 std::shared_ptr<const Grid<mask_t>> mask_ptr,
                 cell_t type);
    auto get_velocity_grid() -> Grid<Vec2D<float>>*;
    auto get_density_grid()  -> Grid<float>*;
    auto get_type_grid()     -> Grid<cell_t>*;
    void write_data(std::ostream& dest);
    void read_data(std::istream& src);
    void stream();
    void collide();

    Grid<Cell> src;
    Grid<Cell> dest;
};
}
#endif // FELDRAND__MRT_LBM_HPP
