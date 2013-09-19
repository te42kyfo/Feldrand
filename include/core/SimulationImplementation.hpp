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

#ifndef FELDRAND__SIMULATION_IMPLEMENTATION_HPP
#define FELDRAND__SIMULATION_IMPLEMENTATION_HPP

#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>
#include <string>
#include <iterator>
#include <memory>
#include <istream>
#include "Simulation.hpp"
#include "core/Grid.hpp"
#include "core/Vec2D.hpp"

namespace std { class thread; }

namespace Feldrand {

	struct Cell {
		float NW, N, NE;
		float W,  C,  E;
		float SW, S, SE;
		cell_t type;
	};

	std::ostream& operator<<(std::ostream& dest, const Cell& cell);
	std::istream& operator>>(std::istream& src, Cell& cell);

	
	class Simulation::SimulationImplementation {
	protected:
		/* These funcitons correspond to the Simulation API */
		SimulationImplementation();
		SimulationImplementation(double width /*in meters*/,
								 double height /*in meters*/,
								 size_t grid_width,
								 size_t grid_height);
	public: // TODO copy operator
		SimulationImplementation(const SimulationImplementation& other);
		virtual ~SimulationImplementation();

		template<typename T>
		void action(Action what, T data);
		void action(Action what);

		template<typename T>
		auto get(Data what) -> T;

		void beginMultiple();
		void endMultiple();

	private:
		void loop();
		void advance();
		void handle_requests();
		void do_pause();
		void do_run();
		void do_steps(size_t steps);

		auto get_width()         -> double;
		auto get_height()        -> double;
		auto get_gridWidth()     -> size_t;
		auto get_gridHeight()    -> size_t;
		auto get_timestep_id()   -> size_t;

	protected:
		/* interface for iterative fluid solvers */
		virtual void init() = 0;
		virtual void one_iteration() = 0;
		virtual void do_clear() = 0;
		virtual void do_draw(int x, int y,
							 std::shared_ptr<const Grid<mask_t>> mask_ptr,
							 cell_t type) = 0;
		virtual auto get_velocity_grid() -> Grid<Vec2D<float>>* = 0;
		virtual auto get_density_grid()  -> Grid<float>* = 0;
		virtual auto get_type_grid()     -> Grid<cell_t>* = 0;
		virtual void write_data(std::ostream& dest) = 0;
		virtual void read_data(std::istream& src) = 0;

	protected:
		double width;
		double height;
		size_t gridWidth;
		size_t gridHeight;
		double kinematic_viscosity;
		double density;
		/* 1.0 is realtime, 2.0 is twice as fast, 0.5 is half as fast*/
		double speed;
		size_t ts_id;

		std::thread* work_thread;

		std::mutex rw_mutex;
		std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;

		std::mutex todo_queue_mutex;
		std::queue<std::function<void()>> todo_queue;

		/* tell the simulation to pause or run */
		bool pause;
		/* set to true before deletion to collect the work_thread */
		bool join;
		/* how many immediate simulation steps shall be done */
		size_t stepsToDo;

		friend std::ostream&
		operator<<(std::ostream &dest,
				   Simulation::SimulationImplementation& sim);
		friend std::istream&
		operator>>(std::istream &src,
				   Simulation::SimulationImplementation& sim);
	};

	template<>
	void Simulation::SimulationImplementation::
	action<Simulation::draw_data&>(Action what, Simulation::draw_data& data);
	template<>
	void Simulation::SimulationImplementation::
	action<size_t>(Action what, size_t data);

	template<typename T>
	auto Simulation::SimulationImplementation::
	get(Data what) -> T {
		throw std::runtime_error("Simulation::get() called with invalid "
								 "template type");
	}
	template<>
	auto Simulation::SimulationImplementation::
	get<double>(Data what) -> double;
	template<>
	auto Simulation::SimulationImplementation::
	get<size_t>(Data what) -> size_t;
	template<>
	auto Simulation::SimulationImplementation::
	get<Grid<Vec2D<float>>*>(Data what) -> Grid<Vec2D<float>>*;
	template<>
	auto Simulation::SimulationImplementation::
	get<Grid<float>*>(Data what) -> Grid<float>*;
	template<>
	auto Simulation::SimulationImplementation::
	get<Grid<cell_t>*>(Data what) -> Grid<cell_t>*;
}

#endif // FELDRAND__SIMULATION_IMPLEMENTATION_HPP
