/* Copyright (C) 2013  Dominik Ernst

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

#include "core/BGK_OCL.hpp"

using namespace std;

namespace Feldrand {



	namespace {
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


	}




	BGK_OCL::BGK_OCL()
		: SimulationImplementation(0.0, 0.0, 0, 0) {}

	BGK_OCL::BGK_OCL(double width, double height,
					 size_t grid_width, size_t grid_height)
		: SimulationImplementation(width, height, grid_width, grid_height),
		  vel( grid_width*grid_height*2 )
	{
		if( !OpenCLHelper::isOpenCLAvailable() ) {
			cout << "opencl library not found" << endl;
			exit(-1);
		}
		cout << "found opencl library" << endl;

		OpenCLHelper cl(0);
		kernel = cl.buildKernel("./src/core/getVelocity.cl", "getVelocity");
		


		kernel->output( vel.size(), vel.data() );
		kernel->input( (int) grid_width );
		kernel->input( (int) grid_height );
		size_t global[2];
		global[0] = OpenCLHelper::roundUp(64, grid_width);
		global[1] = OpenCLHelper::roundUp(64, grid_height);
		size_t local[] = { 16, 16 };
		
		kernel->run(2, global, local );
		
				
		do_clear();
	}

	BGK_OCL::BGK_OCL(BGK_OCL& other)
		: SimulationImplementation(other)
	{
		// TODO copy data
	}


	BGK_OCL::~BGK_OCL() {}

	void BGK_OCL::one_iteration() {
    
	}

	void BGK_OCL::do_clear() {
		
	}

	void BGK_OCL::do_draw(int x, int y,
						  shared_ptr<const Grid<mask_t>> mask_ptr,
						  cell_t type) {

	}

	auto BGK_OCL::get_velocity_grid() -> Grid<Vec2D<float>>* {
		Grid<Vec2D<float>>* g(new Grid<Vec2D<float>>(gridWidth, gridHeight));
		for(size_t iy = 0; iy < gridHeight; ++iy) {
			for(size_t ix = 0; ix < gridWidth; ++ix) {
				(*g)(ix, iy) = Vec2D<float> { vel[iy * gridWidth*2 +ix*2],
											  vel[iy * gridWidth*2 +ix*2 +1] };
			}
		}
		return g;
	}

	auto BGK_OCL::get_density_grid()  -> Grid<float>* {
		Grid<float>* g(new Grid<float>(gridWidth, gridHeight));
		for(size_t iy = 0; iy < gridHeight; ++iy) {
			for(size_t ix = 0; ix < gridWidth; ++ix) {
				(*g)(ix, iy) = 1.0 / vel[iy * gridWidth*2 +ix*2];
			}
		}
		return g;
	}

	auto BGK_OCL::get_type_grid()     -> Grid<cell_t>* {
		Grid<cell_t>* g(new Grid<cell_t>(gridWidth, gridHeight));
		for(size_t iy = 0; iy < gridHeight; ++iy) {
			for(size_t ix = 0; ix < gridWidth; ++ix) {
				(*g)(ix, iy) = cell_t::FLUID;
			}
		}
		return g;
	}

	void BGK_OCL::write_data(std::ostream& dest) {
	
	}

	void BGK_OCL::read_data(std::istream& src) {
	
	}



}
