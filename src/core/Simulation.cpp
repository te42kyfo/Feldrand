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

#include <cmath>
#include <iostream>
#include <fstream>
#include <iterator>
#include <cstring>
#include "core/Simulation.hpp"
#include "core/SimulationImplementation.hpp"
using namespace std;

namespace Feldrand {

Simulation Simulation::create_dwdhgt(double width,
                                     double height,
                                     size_t total_points) {
    size_t grid_width
        = (size_t)ceil(sqrt( (width  / height) * (double)total_points));
    size_t grid_height
        = (size_t)ceil(sqrt( (height /  width) * (double)total_points));
    return Simulation{width, height, grid_width, grid_height};
}

Simulation Simulation::create_dwdhgw(double width,
                                     double height,
                                     size_t grid_width) {
    size_t grid_height = (size_t)(height / width) * grid_width;
    return Simulation{width, height, grid_width, grid_height};
}

Simulation Simulation::create_dwdhgh(double width,
                                     double height,
                                     size_t grid_height) {
    size_t grid_width = (width / height) * grid_height;
    return Simulation{width, height, grid_width, grid_height};
}

Simulation::Simulation(double width /*in meters*/,
                       double height /*in meters*/,
                       size_t grid_width,
                       size_t grid_height)
    :impl(new SimulationImplementation(width, height,
                                       grid_width,
                                       grid_height)) {}

Simulation::Simulation()
    :impl(new SimulationImplementation()) {}

Simulation::Simulation(Simulation&& other)
    :impl(other.impl) {
    other.impl = NULL;
}

Simulation::Simulation(const Simulation& other)
    :impl(new SimulationImplementation(*(other.impl))) {}

Simulation::~Simulation() {
    delete impl;
}

void Simulation::action(Action what) {
    impl->action(what);
}

template<> void
Simulation::action<Simulation::draw_data&>(Action what,
                                           Simulation::draw_data& data) {
    impl->action<Simulation::draw_data&>(what, data);
}

template<> void
Simulation::action<size_t>(Action what, size_t data) {
    impl->action<size_t>(what, data);
}

template<>
auto Simulation::get(Data what) -> double {
    return impl->get<double>(what);
}

template<>
auto Simulation::get(Data what) -> size_t {
    return impl->get<size_t>(what);
}

template<>
auto Simulation::get(Data what) -> Grid<Vec2D<float>>* {
    return impl->get<Grid<Vec2D<float>>*>(what);
}

template<>
auto Simulation::get(Data what) -> Grid<float>* {
    return impl->get<Grid<float>*>(what);
}

template<>
auto Simulation::get(Data what) -> Grid<cell_t>* {
    return impl->get<Grid<cell_t>*>(what);
}

void Simulation::beginMultiple() {
    impl->beginMultiple();
}

void Simulation::endMultiple() {
    impl->endMultiple();
}

string Simulation::toString(Data what) {
    switch(what) {
    case Data::width: return string("width");
    case Data::height: return string("height");
    case Data::gridWidth: return string("gridWidth");
    case Data::gridHeight: return string("gridHeight");
    case Data::timestep_id: return string("timestep_id");
    case Data::velocity_grid: return string("velocity_grid");
    case Data::density_grid: return string("density_grid");
    case Data::type_grid: return string("type_grid");
    default: break;
    }
    return string("unknown");
}

string Simulation::toString(Simulation::Action  what) {
    switch(what) {
    case Action::pause: return string("pause");
    case Action::run: return string("run");
    case Action::clear: return string("clear");
    case Action::draw: return string("draw");
    case Action::steps: return string("steps");
    default: break;
    }
    return string("unknown");
}

std::ostream& operator<<(std::ostream &dest,
                         Simulation& sim) {
    dest << *(sim.impl);
    return dest;
}

std::istream& operator>>(std::istream &src,
                         Simulation& sim) {
    src >> *(sim.impl);
    return src;
}
}
