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

#ifndef FELDRAND__SIMULATION_HPP
#define FELDRAND__SIMULATION_HPP

/* this is the main class of the
 * F ancy
 *  E extensible
 *   L ightweight
 *    D deterministic
 *     R realtime
 *      A dvanced
 *       N umerical
 *        D emonstrator
 *
 * the FELDRAND is a 2-dimensional real time fluid simulator with focus on
 * simplicity and speed. Internal it uses a Lattice-Boltzmann simulation which
 * allows it to handle even complicated geometries.
 *
 * For questions and feedback please contact:
 * - The developer Marco Heisig (marco.heisig@fau.de)
 * - The Chair for System Simulation of the FAU-Erlangen-Nuernberg
 *   (www10.informatik.uni-erlangen.de/en)
 */

#include <cstddef>
#include <stdexcept>
#include <memory>
#include <string>
#include "core/Grid.hpp"
#include "core/Vec2D.hpp"

namespace Feldrand {

// class Simulation;
// std::ostream& operator<<(std::ostream &dest, const Simulation& sim);
// std::istream& operator>>(std::istream &src, Simulation& sim);

enum struct cell_t {
    OBSTACLE,
    FLUID,
    CONSTANT
};

enum struct mask_t {
    MODIFY,
    IGNORE
};

class Simulation {
public:
    /* Each Feldrand::Simulation is performed on a rectangular domain.  The
     * underlying compute grid must have the same ratio of width to height as
     * the domain itself, therefore the parameters width, height, grid_width
     * and grid_height are implicitly computed in each of the following
     * methods. To understand the naming convention of these methods simply
     * replace
     * d = domain, in meters
     * w = width
     * h = height
     * g = grid
     * t = total_points
     * The simulation is in paused state upon construction. Call run() to
     * begin and pause() to pause again. */
    static Simulation create_dwdhgt(double domain_width, double domain_height,
                                    size_t total_points);
    static Simulation create_dwdhgw(double domain_width, double domain_height,
                                    size_t grid_width);
    static Simulation create_dwdhgh(double domain_width, double domain_height,
                                    size_t grid_heigth);
protected:
    Simulation(double domain_width, double domain_height,
               size_t grid_width, size_t grid_height);
public:
    Simulation();
    Simulation(Simulation&& other);
    Simulation(const Simulation& other);
    ~Simulation();

public:
    /* These orders can be used as argument in simulation.action() some of
     * them require additional data to be passed. */
    enum struct Action {
        pause,
        run,
        clear,
        draw,    // requires data = draw_data&
        steps    // requires data = size_t
    };

    struct draw_data {
        int x; int y;
        std::shared_ptr<const Grid<mask_t>> mask_ptr;
        cell_t type;
    };

    /* Make the simulation to perform an action. */
    template<typename T>
    void action(Action what, T data);
    void action(Action what);

    /* These elements can be used as argument for a Simulation's get() method
     * to specify the desired value. */
    enum struct Data {
        width,         // -> double
        height,        // -> double
        gridWidth,     // -> size_t
        gridHeight,    // -> size_t
        timestep_id,   // -> size_t
        velocity_grid, // -> Grid<Vec2D<float>>*
        density_grid,  // -> Grid<float>*
        type_grid      // -> Grid<cell_t>*
    };

    /* Request some data from the Simulation. Calls to this function may block
     * for at most one complete simulation timestep. You might use std::async
     * to avoid this. */
    template<typename T>
    auto get(Data what) -> T;

    /* The Simulations methods do not guarantee that successive requests apply
     * to the same timestep.  If this behaviour is desired, put all those
     * calls in a beginMultiple(); ... endMultiple(); block. */
    void beginMultiple();
    void endMultiple();

    class SimulationImplementation;
private:
    SimulationImplementation* impl;
    friend std::ostream& operator<<(std::ostream &dest,
                                    Simulation& sim);
    friend std::istream& operator>>(std::istream &src,
                                    Simulation& sim);
};

/* Tiny helpers to read or write the Simulation's enums */
std::ostream& operator<<(std::ostream& dest, const Simulation::Data& what);
std::ostream& operator<<(std::ostream& dest, const Simulation::Action& what);
std::ostream& operator<<(std::ostream& dest, const cell_t& type);
std::istream& operator>>(std::istream& src, Simulation::Data& what);
std::istream& operator>>(std::istream& src, Simulation::Action& what);
std::istream& operator>>(std::istream& src, cell_t& type);

template<> void
Simulation::action<size_t>(Action what, size_t data);
template<> void
Simulation::action<Simulation::draw_data&>(Action what, Simulation::draw_data& data);

template<> auto
Simulation::get<double>(Data what) -> double;
template<> auto
Simulation::get<size_t>(Data what) -> size_t;
template<> auto
Simulation::get<Grid<Vec2D<float>>*>(Data what) -> Grid<Vec2D<float>>*;
template<> auto
Simulation::get<Grid<float>*>(Data what) -> Grid<float>*;
template<> auto
Simulation::get<Grid<cell_t>*>(Data what) -> Grid<cell_t>*;

/* If the template type of action() or get() is none of the above
 * ones, static_assert will inform you at compile time. */
template<typename T>
void Simulation::action(Action what, T data) {
    const size_t impossible_size = size_t(0) - size_t(1);
    static_assert(sizeof(T) != impossible_size, // TODO any better ideas?
                  "You use action() with an invalid argument type! "
                  "See the Simulation.hpp header for allowed types.");
}
template<typename T>
auto Simulation::get(Data what) -> T {
    const size_t impossible_size = size_t(0) - size_t(1);
    static_assert(sizeof(T) != impossible_size, // TODO any better ideas?
                  "You use get() with an invalid return type! "
                  "See the Simulation.hpp header for allowed types.");
}
}
#endif // FELDRAND_SIMULATION_HPP
