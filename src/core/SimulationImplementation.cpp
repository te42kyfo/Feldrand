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

#include <thread>
#include <future>
#include <mutex>
#include <cmath>
#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <functional>
#include "core/SimulationImplementation.hpp"
#include "config.hpp"

using namespace std;

namespace Feldrand {

std::ostream& operator<<(std::ostream& dest, const cell_t& type) {
    dest << static_cast<int>(type) << "\n";
    return dest;
}
std::istream& operator>>(std::istream& src, cell_t& type) {
    int i;
    src >> i;
    type = static_cast<cell_t>(i);
    return src;
}
std::ostream& operator<<(std::ostream& dest, const Cell& cell) {
    dest << cell.NW << " " << cell.N << " " << cell.NE << " ";
    dest <<  cell.W << " " << cell.C << " " <<  cell.E << " ";
    dest << cell.SW << " " << cell.S << " " << cell.SE << " ";
    dest << cell.type << "\n";
    return dest;
}
std::istream& operator>>(std::istream& src, Cell& cell) {
    src >> cell.NW >> cell.N >> cell.NE;
    src >>  cell.W >> cell.C >>  cell.E;
    src >> cell.SW >> cell.S >> cell.SE;
    src >> cell.type;
    return src;
}

Simulation::SimulationImplementation::
SimulationImplementation(double width,
                         double height,
                         size_t grid_width,
                         size_t grid_height)
    : width(width), height(height),
      gridWidth(grid_width),
      gridHeight(grid_height),
      kinematic_viscosity(1.0),
      speed(1.0),
      ts_id(0),
      pause(true),
      join(false),
      stepsToDo(0),
      src(gridWidth, gridHeight),
      dest(gridWidth, gridHeight)
{
    /* clear() does my initialisation of the grids */
    do_clear();
    work_thread
        = new thread{&Simulation::SimulationImplementation::loop, this};
}

Simulation::SimulationImplementation::
SimulationImplementation()
    : width(0.0), height(0.0),
      gridWidth(0),
      gridHeight(0),
      kinematic_viscosity(0.0),
      speed(0.0),
      ts_id(0),
      pause(true),
      join(false),
      stepsToDo(0),
      src(gridWidth, gridHeight),
      dest(gridWidth, gridHeight)
{
    work_thread
        = new thread{&Simulation::SimulationImplementation::loop, this};
}

Simulation::SimulationImplementation::
SimulationImplementation(const SimulationImplementation& other)
    : width(other.width), height(other.height),
      gridWidth (other.gridWidth),
      gridHeight(other.gridHeight),
      kinematic_viscosity(other.kinematic_viscosity),
      speed(other.speed),
      ts_id(other.ts_id),
      pause(other.pause),
      join(other.join),
      stepsToDo(other.stepsToDo),
      src(other.src),
      dest(other.dest)
{
    /* clear() does my initialisation of the grids */
    do_clear();
    work_thread
        = new thread{&Simulation::SimulationImplementation::loop, this};
}

Simulation::SimulationImplementation::
~SimulationImplementation() {
    join = true;
    work_thread->join();
    delete work_thread;
}

void Simulation::SimulationImplementation::
action(Action what) {
    lock_guard<mutex> lock(todo_queue_mutex);
    switch(what) {
    case Action::pause:
        do_pause();
        break;
    case Action::run:
        do_run();
        break;
    case Action::clear:
        todo_queue.push([this]{do_clear();});
        break;
    default:
        throw runtime_error(string("invalid Action or type ") + Simulation::toString(what));
    }
}

template<>
void Simulation::SimulationImplementation::
action(Action what, Simulation::draw_data& data) {
    lock_guard<mutex> lock(todo_queue_mutex);
    switch(what) {
    case Action::draw:
        todo_queue.push(bind([this]
                             (int x, int y,
                              shared_ptr<const Grid<mask_t>> mask_ptr,
                              cell_t type) {
                                 do_draw(x, y, mask_ptr, type);
                             },
                             data.x, data.y,
                             data.mask_ptr, data.type));
        break;
    default:
        throw runtime_error(string("invalid Action or type ") + Simulation::toString(what));
    }
}

template<>
void Simulation::SimulationImplementation::
action(Action what, size_t data) {
    switch(what) {
    case Action::steps:
        todo_queue.push(bind([this] (size_t steps) {
                                 do_steps(steps);
                             }, data));
        break;
    default:
        throw runtime_error(string("invalid Action or type ") + Simulation::toString(what));
    }
}


template<>
auto Simulation::SimulationImplementation::
get(Data what) -> double {
    lock_guard<mutex> lock(todo_queue_mutex);
    switch(what) {
    case Data::width:
        return get_width();
        break;
    case Data::height:
        return get_height();
        break;
    default:
        throw runtime_error(string("invalid Data or type ") + Simulation::toString(what));
    }
    return 0.0;
}

template<>
auto Simulation::SimulationImplementation::
get(Data what) -> size_t {
    lock_guard<mutex> lock(todo_queue_mutex);

    switch(what) {
    case Data::gridWidth:
        return get_gridWidth();
        break;
    case Data::gridHeight:
        return get_gridHeight();
        break;
    case Data::timestep_id:
        return get_timestep_id();
        break;
    default:
        throw runtime_error(string("invalid Data or type ") + Simulation::toString(what));
    }
    return 0;
}

template<>
auto Simulation::SimulationImplementation::
get(Data what) -> Grid<Vec2D<float>>* {
    todo_queue_mutex.lock();
    auto p = new promise<Grid<Vec2D<float>>*>();
    future<Grid<Vec2D<float>>*> f = p->get_future();

    switch(what) {
    case Data::velocity_grid:
        todo_queue.push(bind([this](promise<Grid<Vec2D<float>>*>* p) {
                    p->set_value(get_velocity_grid());
                    delete p;
                }, p));
        break;
    default:
        delete p;
        todo_queue_mutex.unlock();
        throw runtime_error(string("invalid Data or type ") + Simulation::toString(what));
    }
    todo_queue_mutex.unlock();
    return f.get();
}

template<>
auto Simulation::SimulationImplementation::
get(Data what) -> Grid<float>* {
    todo_queue_mutex.lock();
    auto p = new promise<Grid<float>*>();
    future<Grid<float>*> f = p->get_future();

    switch(what) {
    case Data::density_grid:
        todo_queue.push(bind([this](promise<Grid<float>*>* p) {
                    p->set_value(get_density_grid());
                    delete p;
                }, p));
        break;
    default:
        delete p;
        todo_queue_mutex.unlock();
        throw runtime_error(string("invalid Data or type ") + Simulation::toString(what));
    }
    todo_queue_mutex.unlock();
    return f.get();
}

template<>
auto Simulation::SimulationImplementation::
get(Data what) -> Grid<cell_t>* {
    todo_queue_mutex.lock();
    auto p = new promise<Grid<cell_t>*>();
    future<Grid<cell_t>*> f = p->get_future();

    switch(what) {
    case Data::velocity_grid:
        cout << "queuing one" << endl;
        todo_queue.push(bind([this](promise<Grid<cell_t>*>* p) {
                    p->set_value(get_type_grid());
                    delete p;
                }, p));
        break;
    default:
        delete p;
        todo_queue_mutex.unlock();
        throw runtime_error(string("invalid Data or type ") + Simulation::toString(what));
    }
    todo_queue_mutex.unlock();
    return f.get();
}

void Simulation::SimulationImplementation::
beginMultiple() {
    lock_guard<mutex> lock(todo_queue_mutex);
    //multiple_mutex.lock();
}

void Simulation::SimulationImplementation::
endMultiple() {
    lock_guard<mutex> lock(todo_queue_mutex);
    //multiple_mutex.unlock();
}

void Simulation::SimulationImplementation::
loop() {
    while(!join) {

        collide();
        stream();
        Grid<Cell>::swap(src, dest);
        advance();
    }
}

void Simulation::SimulationImplementation::
advance() {
    ++ts_id;

    handle_requests();

    using namespace std::chrono;
    microseconds given_time(2000); // TODO calculate this properly
    microseconds compute_time = high_resolution_clock::now() - timestamp;

    rw_mutex.unlock();
    if(compute_time < given_time)
        this_thread::sleep_for(given_time - compute_time);
    while(pause) this_thread::sleep_for(given_time);
    rw_mutex.lock();
    timestamp = high_resolution_clock::now();
}

/* The streaming step of the MRT-LBM simulation. The values of each fluid
 * cell are exchanged with their neighbors or reflected should the
 * neighbor be an obstacle cell */
void Simulation::SimulationImplementation::
stream() {
    /* Instead of reflecting the cells value next to an obstacle, we
     * copy the value in the opposite entry of the obstacle cell. Afterwards
     * all cells can simply exchange values without any conditionals, the
     * net effect is the same. */
    for(size_t iy = 1; iy < gridHeight - 1; ++iy) {
        for(size_t ix = 1; ix < gridWidth - 1; ++ix) {
            if(src(ix, iy).type != cell_t::FLUID) continue;
            Cell& NW = src(ix - 1, iy + 1);
            Cell& N  = src(ix    , iy + 1);
            Cell& NE = src(ix + 1, iy + 1);
            Cell& W  = src(ix - 1, iy    );
            Cell& C  = src(ix    , iy    );
            Cell& E  = src(ix + 1, iy    );
            Cell& SW = src(ix - 1, iy - 1);
            Cell& S  = src(ix    , iy - 1);
            Cell& SE = src(ix + 1, iy - 1);

            /* noslip boundaries */
            if(cell_t::OBSTACLE == NW.type) NW.SE = C.NW;
            if(cell_t::OBSTACLE ==  N.type)  N.S  = C.N;
            if(cell_t::OBSTACLE == NE.type) NE.SW = C.NE;
            if(cell_t::OBSTACLE ==  W.type)  W.E  = C.W;
            if(cell_t::OBSTACLE ==  E.type)  E.W  = C.E;
            if(cell_t::OBSTACLE == SW.type) SW.NE = C.SW;
            if(cell_t::OBSTACLE ==  S.type)  S.N  = C.S;
            if(cell_t::OBSTACLE == SE.type) SE.NW = C.SE;
        }
    }
    /* exchanging values */
    for(size_t iy = 1; iy < gridHeight - 1; ++iy) {
        for(size_t ix = 1; ix < gridWidth - 1; ++ix) {
            if(src(ix, iy).type != cell_t::FLUID) continue;
            dest(ix, iy).NW = src(ix + 1, iy + 1).NW;
            dest(ix, iy).N  = src(ix    , iy + 1).N ;
            dest(ix, iy).NE = src(ix - 1, iy + 1).NE;
            dest(ix, iy).W  = src(ix + 1, iy    ).W ;
            dest(ix, iy).C  = src(ix    , iy    ).C ;
            dest(ix, iy).E  = src(ix - 1, iy    ).E ;
            dest(ix, iy).SW = src(ix + 1, iy - 1).SW;
            dest(ix, iy).S  = src(ix    , iy - 1).S ;
            dest(ix, iy).SE = src(ix - 1, iy - 1).SE;
        }
    }
}

/* the collision step of the MRT-LBM simulation. The moments of each
 * cell are calculated and individually relaxed towards equilibrium. */
void Simulation::SimulationImplementation::
collide() {
    float omega = 1.9;
    for(size_t iy = 0; iy < gridHeight; ++iy) {
        for(size_t ix = 0; ix < gridWidth; ++ix) {
            Cell& cell = src(ix, iy);
            if(cell.type != cell_t::FLUID) {
                // TODO updata vel and dens with newer moments
                continue;
            }
            float p1 = 1.63;
            float p2 = 1.14;
            float p4 = 1.9;
            float p6 = 1.92;
            float p7 = omega;
            float p8 = omega;

            // calculation of the moments
            float m0; // the density
            float m1; // the energy
            float m2; // energy squared
            float m3; // momentum x
            float m4; // heatflow x
            float m5; // momentum y
            float m6; // heatflow y
            float m7; // diagonal stress
            float m8; // off-diagonal stress
            m0 =         cell.NW +       cell.N +       cell.NE
                +       cell.W  +       cell.C +       cell.E
                +       cell.SW +       cell.S +       cell.SE;

            m1 =   2.0 * cell.NW -       cell.N + 2.0 * cell.NE
                -       cell.W  - 4.0 * cell.C -       cell.E
                + 2.0 * cell.SW -       cell.S + 2.0 * cell.SE;

            m2 =         cell.NW - 2.0 * cell.N +       cell.NE
                - 2.0 * cell.W  + 4.0 * cell.C - 2.0 * cell.E
                +       cell.SW - 2.0 * cell.S +       cell.SE;

            m3 = -       cell.NW +          0.0 +       cell.NE
                -       cell.W  +          0.0 +       cell.E
                -       cell.SW +          0.0 +       cell.SE;

            m4 = -       cell.NW +          0.0 +       cell.NE
                + 2.0 * cell.W  +          0.0 - 2.0 * cell.E
                -       cell.SW +          0.0 +       cell.SE;

            m5 =         cell.NW +       cell.N +       cell.NE
                +           0.0 +          0.0 +           0.0
                -       cell.SW -       cell.S -       cell.SE;

            m6 =         cell.NW - 2.0 * cell.N +       cell.NE
                +           0.0 +          0.0 +           0.0
                -       cell.SW + 2.0 * cell.S -       cell.SE;

            m7 =             0.0 -       cell.N +           0.0
                +       cell.W  +          0.0 +       cell.E
                +           0.0 -       cell.S +           0.0;

            m8 = -       cell.NW +          0.0 +       cell.NE
                +           0.0 +          0.0 +           0.0
                +       cell.SW +          0.0 -       cell.SE;

            float vSquared = m3 * m3 + m5 * m5;

            // moment relaxation
            m1 -= p1 * (m1 + (2.0 * m0 - 3.0 * vSquared));
            m2 -= p2 * (m2 - (m0 - 3.0 * vSquared));
            m4 -= p4 * (m4 + m3);
            m6 -= p6 * (m6 + m5);
            m7 -= p7 * (m7 - (m3 * m3 - m5 * m5));
            m8 -= p8 * (m8 - m3 * m5);

            // back transformation of the moments
            m0 = 4.0 * m0;
            m3 = 6.0 * m3;
            m4 = 3.0 * m4;
            m5 = 6.0 * m5;
            m6 = 3.0 * m6;
            m7 = 9.0 * m7;
            m8 = 9.0 * m8;
            cell.NW = (m0 +2.0*m1 +    m2 -m3 -    m4 +m5 +    m6     -m8)/36.0;
            cell.N  = (m0 -    m1 -2.0*m2             +m5 -2.0*m6 -m7    )/36.0;
            cell.NE = (m0 +2.0*m1 +    m2 +m3 +    m4 +m5 +    m6     +m8)/36.0;
            cell.W  = (m0 -    m1 -2.0*m2 -m3 +2.0*m4             +m7    )/36.0;
            cell.C  = (m0 -4.0*m1 +4.0*m2                                )/36.0;
            cell.E  = (m0 -    m1 -2.0*m2 +m3 -2.0*m4             +m7    )/36.0;
            cell.SW = (m0 +2.0*m1 +    m2 -m3 -    m4 -m5 -    m6     +m8)/36.0;
            cell.S  = (m0 -    m1 -2.0*m2             -m5 +2.0*m6 -m7    )/36.0;
            cell.SE = (m0 +2.0*m1 +    m2 +m3 +    m4 -m5 -    m6     -m8)/36.0;
        }
    }
}

void Simulation::SimulationImplementation::
handle_requests() {
    //multiple_mutex.lock();
    lock_guard<mutex> lock1(todo_queue_mutex);
    while(!todo_queue.empty()) {
        todo_queue.front()();
        todo_queue.pop();
    }
    //multiple_mutex.unlock();
}

void Simulation::SimulationImplementation::
do_pause() {
    pause = true;
}

void Simulation::SimulationImplementation::
do_run() {
    pause = false;
}

void Simulation::SimulationImplementation::
do_clear() {
    for(size_t iy = 0; iy < gridHeight; ++iy) {
        for(size_t ix = 0; ix < gridWidth; ++ix) {
            src(ix, iy) = fluid;
            dest(ix, iy) = fluid;
        }
    }
    // TODO remove this fun hack that makes clear initialize a wind tunnel
    for(size_t iy = 0; iy < gridHeight; ++iy) {
        size_t ix = 0;
        src(ix, iy) = source;
        dest(ix, iy) = source;
    }
    for(size_t iy = 0; iy < gridHeight; ++iy) {
        size_t ix = gridWidth - 1;
        src(ix, iy) = drain;
        dest(ix, iy) = drain;
    }
}

void Simulation::SimulationImplementation::
do_draw(int x, int y,
        shared_ptr<const Grid<mask_t>> mask_ptr,
        cell_t type) {
    int cx = x;
    int cy = y;
    const Grid<mask_t>& mask = *(mask_ptr);

    int upper_left_x = cx - (mask.x() / 2);
    int upper_left_y = cy - (mask.y() / 2);
    for(size_t iy = 0; iy < mask.y(); ++iy) {
        for(size_t ix = 0; ix < mask.x(); ++ix) {
            int sx = upper_left_x + ix;
            int sy = upper_left_y + iy;
            if(sx < 0 || sx >= (int)gridWidth ||
               sy < 0 || sy >= (int)gridHeight) continue;

            if(mask_t::IGNORE == mask(ix, iy)) continue;
            src(sx, sy).type = type; // TODO handle pressure
            dest(sx, sy).type = type;
        }
    }
}

void Simulation::SimulationImplementation::
do_steps(size_t steps) {
    stepsToDo += steps;
}

void Simulation::SimulationImplementation::
do_save(ostream* destination) {
    ostream& dest = *destination;
    dest << "FELDRAND SIMULATION\n";
    dest << "0\n";
    dest << width << "\n";
    dest << height << "\n";
    dest << gridWidth << "\n";
    dest << gridHeight << "\n";
    dest << kinematic_viscosity << "\n";
    dest << speed << "\n";
    dest << ts_id << "\n";
    for(size_t iy = 0; iy < gridHeight; ++iy) {
        for(size_t ix = 0; ix < gridWidth; ++ix) {
            const Cell& cell = src(ix, iy);
            dest << static_cast<int>(cell.type) << "\n";
            dest << cell.NW   << "\n";
            dest << cell.N    << "\n";
            dest << cell.NE   << "\n";
            dest << cell.W    << "\n";
            dest << cell.C    << "\n";
            dest << cell.E    << "\n";
            dest << cell.SW   << "\n";
            dest << cell.S    << "\n";
            dest << cell.SE   << "\n";
        }
    }
}

size_t
Simulation::SimulationImplementation::
get_gridWidth() {
    return gridWidth;
}

size_t
Simulation::SimulationImplementation::
get_gridHeight() {
    return gridHeight;
}

double
Simulation::SimulationImplementation::
get_width() {
    return width;
}

double
Simulation::SimulationImplementation::
get_height() {
    return height;
}

size_t
Simulation::SimulationImplementation::
get_timestep_id() {
    return ts_id;
}

Grid<Vec2D<float>>*
Simulation::SimulationImplementation::
get_velocity_grid() {
    Grid<Vec2D<float>>* g(new Grid<Vec2D<float>>(gridWidth, gridHeight));
    for(size_t iy = 0; iy < gridHeight; ++iy) {
        for(size_t ix = 0; ix < gridWidth; ++ix) {
            const Cell& cell = src(ix, iy);
            float vx, vy;
            vx = -       cell.NW +          0.0 +       cell.NE
                -       cell.W  +          0.0 +       cell.E
                -       cell.SW +          0.0 +       cell.SE;

            vy =         cell.NW +       cell.N +       cell.NE
                +           0.0 +          0.0 +           0.0
                -       cell.SW -       cell.S -       cell.SE;

            (*g)(ix, iy) = {vx, vy};
        }
    }
    return g;
}

Grid<float>*
Simulation::SimulationImplementation::
get_density_grid() {
    Grid<float>* g(new Grid<float>(gridWidth, gridHeight));
    for(size_t iy = 0; iy < gridHeight; ++iy) {
        for(size_t ix = 0; ix < gridWidth; ++ix) {
            const Cell& cell = src(ix, iy);
            float d =    cell.NW +       cell.N +       cell.NE
                +       cell.W  +       cell.C +       cell.E
                +       cell.SW +       cell.S +       cell.SE;
            (*g)(ix, iy) = d;
        }
    }
    return g;
}

Grid<cell_t>*
Simulation::SimulationImplementation::
get_type_grid() {
    Grid<cell_t>* g(new Grid<cell_t>(gridWidth, gridHeight));
    for(size_t iy = 0; iy < gridHeight; ++iy) {
        for(size_t ix = 0; ix < gridWidth; ++ix) {
            const Cell& cell = src(ix, iy);
            (*g)(ix, iy) = cell.type;
        }
    }
    return g;
}

std::ostream&
operator<<(std::ostream &dest,
           Simulation::SimulationImplementation& sim) {
    lock_guard<mutex> lock(sim.rw_mutex);
    dest << "Feldrand Simulation\n";
    dest << Feldrand::core_so_version << "\n";
    dest << sim.width << "\n";
    dest << sim.height << "\n";
    dest << sim.gridWidth << "\n";
    dest << sim.gridHeight << "\n";
    dest << sim.kinematic_viscosity << "\n";
    dest << sim.density << "\n";
    dest << sim.speed << "\n";
    dest << sim.ts_id << "\n";
    dest << sim.src << "\n";
    dest << sim.dest << "\n";
    return dest;
}

std::istream&
operator>>(std::istream &src,
           Simulation::SimulationImplementation& sim) {
    lock_guard<mutex> lock(sim.rw_mutex);
    char name[20];
    src.getline(name, 20);
    if(strncmp(name, "Feldrand Simulation", 19))
        throw std::runtime_error("Invalid Simulation file");
    string version;
    src >> version;
    stringstream errmsg;
    errmsg << "Can not open version " << version << " file with a version "
           << core_so_version << " simulation.";
    if(version != core_so_version)
        throw std::runtime_error(errmsg.str());
    src >> sim.width;
    src >> sim.height;
    src >> sim.gridWidth;
    src >> sim.gridHeight;
    src >> sim.kinematic_viscosity;
    src >> sim.density;
    src >> sim.speed;
    src >> sim.ts_id;
    src >> sim.src;
    src >> sim.dest;
    return src;
}
}
