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
      stepsToDo(0)
{
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
      stepsToDo(0)
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
      stepsToDo(other.stepsToDo)
{
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
        throw runtime_error(string("invalid Action or type "));
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
        throw runtime_error(string("invalid Action or type "));
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
        throw runtime_error(string("invalid Action or type "));
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
        throw runtime_error(string("invalid Data or type "));
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
        throw runtime_error(string("invalid Data or type "));
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
        throw runtime_error(string("invalid Data or type "));
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
        throw runtime_error(string("invalid Data or type "));
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
    case Data::type_grid:
        todo_queue.push(bind([this](promise<Grid<cell_t>*>* p) {
                    p->set_value(get_type_grid());
                    delete p;
                }, p));
        break;
    default:
        delete p;
        todo_queue_mutex.unlock();
        throw runtime_error(string("invalid Data or type "));
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

const size_t iters = 5;

void Simulation::SimulationImplementation::
loop() {
	init();
	while(!join) {
		for( size_t n = 0; n < iters; n++) {
			one_iteration();
		}

		advance();
    }
}

void Simulation::SimulationImplementation::
advance() {
    ++ts_id;

    handle_requests();

    using namespace std::chrono;
    microseconds given_time(2000); // TODO calculate this properly
   

	
	milliseconds compute_time =
		duration_cast<milliseconds>(high_resolution_clock::now() - timestamp);
	
	/*std::cout  << "" 
			   <<  (gridHeight*gridWidth*iters) / 
		(compute_time.count()-100) *1.0e-3
		<< "  MLup/s \n";*/
	//std::cout.flush();
	timestamp = high_resolution_clock::now();
	
	

    while(pause) {
        this_thread::sleep_for(given_time);
        handle_requests();
    }
	this_thread::sleep_for( milliseconds(100) );

	



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
do_steps(size_t steps) {
    stepsToDo += steps;
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
    sim.write_data(dest);
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
    sim.read_data(src);
    return src;
}


	std::ostream& operator<<(std::ostream& dest, const Cell& cell) {
		dest << cell.NW << cell.N << cell.NE
			 << cell.W << cell.C << cell.E
			 << cell.SW << cell.S << cell.SE;
		return dest;
	}

	std::istream& operator>>(std::istream& src, Cell& cell) {
		src >> cell.NW >> cell.N >> cell.NE
			>> cell.W >> cell.C >> cell.E
			>> cell.SW >> cell.S >> cell.SE;
		return src;
	}


}
