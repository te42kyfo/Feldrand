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
#include "core/MRT_LBM.hpp"
#include "core/BGK_OCL.hpp"

using namespace std;

namespace Feldrand {

	typedef BGK_OCL SimulationType;

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
		:impl(new SimulationType(width, height,
						  grid_width,
						  grid_height)) {}

	Simulation::Simulation()
		:impl(new SimulationType()) {}

	Simulation::Simulation(Simulation&& other)
		: impl(other.impl) {
		other.impl = nullptr;
	}

	Simulation::Simulation(const Simulation& other) {
		// TODO
		impl = new SimulationType(static_cast<SimulationType&>(*(other.impl)));
	}

	Simulation::~Simulation() {
		delete impl;
	}

	void Simulation::action(Simulation::Action what) {
		impl->action(what);
	}

	template<> void
	Simulation::action<Simulation::draw_data&>(Simulation::Action what,
											   Simulation::draw_data& data) {
		impl->action<Simulation::draw_data&>(what, data);
	}

	template<> void
	Simulation::action<size_t>(Simulation::Action what, size_t data) {
		impl->action<size_t>(what, data);
	}

	template<>
	auto Simulation::get(Simulation::Data what) -> double {
		return impl->get<double>(what);
	}

	template<>
	auto Simulation::get(Simulation::Data what) -> size_t {
		return impl->get<size_t>(what);
	}

	template<>
	auto Simulation::get(Simulation::Data what) -> Grid<Vec2D<float>>* {
		return impl->get<Grid<Vec2D<float>>*>(what);
	}

	template<>
	auto Simulation::get(Simulation::Data what) -> Grid<float>* {
		return impl->get<Grid<float>*>(what);
	}

	template<>
	auto Simulation::get(Simulation::Data what) -> Grid<cell_t>* {
		return impl->get<Grid<cell_t>*>(what);
	}

	void Simulation::beginMultiple() {
		impl->beginMultiple();
	}

	void Simulation::endMultiple() {
		impl->endMultiple();
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

	std::ostream& operator<<(std::ostream& dest, const Simulation::Data& what) {
		switch(what) {
		case Simulation::Data::width: dest << string("width");
		case Simulation::Data::height: dest << string("height");
		case Simulation::Data::gridWidth: dest << string("gridWidth");
		case Simulation::Data::gridHeight: dest << string("gridHeight");
		case Simulation::Data::timestep_id: dest << string("timestep_id");
		case Simulation::Data::velocity_grid: dest << string("velocity_grid");
		case Simulation::Data::density_grid: dest << string("density_grid");
		case Simulation::Data::type_grid: dest << string("type_grid");
		default: break;
		}
		dest << string("unknown");
		return dest;
	}

	std::ostream& operator<<(std::ostream& dest, const Simulation::Action& what) {
		switch(what) {
		case Simulation::Action::pause: dest << string("pause");
		case Simulation::Action::run: dest << string("run");
		case Simulation::Action::clear: dest << string("clear");
		case Simulation::Action::draw: dest << string("draw");
		case Simulation::Action::steps: dest << string("steps");
		default: break;
		}
		dest << string("unknown");
		return dest;
	}

	std::ostream& operator<<(std::ostream& dest, const cell_t& type) {
		dest << static_cast<int>(type) << " ";
		return dest;
	}

	std::istream& operator>>(std::istream& src, Simulation::Data& what) {
		string s;
		src >> s;
		
		// TODO
		return src;
	}

	std::istream& operator>>(std::istream& src, Simulation::Action& what){
		throw std::runtime_error("operator>>(istream, Simulation::Action) not implemented");
		return src;
	}

	std::istream& operator>>(std::istream& src, cell_t& type) {
		int i;
		src >> i;
		type = static_cast<cell_t>(i);
		return src;
	}
}
