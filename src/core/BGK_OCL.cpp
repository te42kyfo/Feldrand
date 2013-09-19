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
#include <sys/types.h>


using namespace std;

namespace Feldrand {



	namespace {
		enum class cell_type : int {
			FLUID = 0,
			NO_SLIP = 1,
			SOURCE = 2
		};

		const float fluid[] = {
			1.0f/36.0f, 1.0f/9.0f, 1.0f/36.0f,
			1.0f/9.0f,  4.0f/9.0f, 1.0f/9.0f,
			1.0f/36.0f, 1.0f/9.0f, 1.0f/36.0f
		};

		const float source[] = {
			1.1f/36.0f, 1.1f/9.0f, 1.1f/36.0f,
			1.1f/9.0f,  3.5f/9.0f, 1.1f/9.0f,
			1.1f/36.0f, 1.1f/9.0f, 1.1f/36.0f
		};

		const float drain[] = {
			0.9f/36.0f, 0.9f/9.0f, 0.9f/36.0f,
			0.9f/9.0f,   4.5f/9.0f, 0.9f/9.0f,
			0.9f/36.0f, 0.9f/9.0f, 0.9f/36.0f
		};

	}




	BGK_OCL::BGK_OCL()
		: SimulationImplementation(0.0, 0.0, 0, 0),
		  getVelocityKernel(NULL),
		  getDensityKernel(NULL),
		  simulationStepKernel(NULL) {}


	BGK_OCL::BGK_OCL(double width, double height,
					 size_t grid_width, size_t grid_height)
		: SimulationImplementation(width, height, grid_width, grid_height),
		  getVelocityKernel(NULL),
		  getDensityKernel(NULL),
		  simulationStepKernel(NULL),
		  vel( grid_width*grid_height*2 )
	{

	}

	BGK_OCL::BGK_OCL(BGK_OCL& other)
		: SimulationImplementation(other), cl(0)
	{
		// TODO copy data
	}


	BGK_OCL::~BGK_OCL() {
		delete getVelocityKernel;
		delete getDensityKernel;
		delete simulationStepKernel;
		delete cl;
	}


	// OpenCL initialization needs to be doen from the same thread that calls
	// the kernels later
	void BGK_OCL::init() {

		if( !OpenCLHelper::isOpenCLAvailable() ) {
			cout << "opencl library not found" << endl;
			exit(-1);
		}

		cl = new OpenCLHelper(0);
		getVelocityKernel = cl->buildKernel("./src/core/getVelocity.cl",
										   "getVelocity");
		getDensityKernel = cl->buildKernel("./src/core/getDensity.cl",
										   "getDensity");
		simulationStepKernel = cl->buildKernel("./src/core/simulationStep.cl",
										   "simulationStep");

		for( size_t i = 0; i < 9; i++) {
			src[i] = cl->arrayFloat(gridWidth*gridHeight);
			src[i]->createOnHost();
			dst[i] = cl->arrayFloat(gridWidth*gridHeight);
			dst[i]->createOnHost();
		}
		flag_field = cl->arrayInt(gridWidth*gridHeight);
		flag_field->createOnHost();

		global_size[0] = OpenCLHelper::roundUp(64, gridWidth);
		global_size[1] = OpenCLHelper::roundUp(64, gridHeight);
		local_size[0] = 16;
		local_size[1] = 16;

		do_clear();

	}


	void BGK_OCL::one_iteration() {
		simulationStepKernel->input( (int) gridWidth);
		simulationStepKernel->input( (int) gridHeight);
		for(size_t i = 0; i < 9; i++) {
			simulationStepKernel->input( src[i] );
		}
		for(size_t i = 0; i < 9; i++) {
			simulationStepKernel->output( dst[i] );
		}
		simulationStepKernel->input( flag_field );

		simulationStepKernel->run(2, global_size, local_size);
		for( size_t i = 0; i < 9; i++) {
			std::swap(src[i], dst[i]);
		}
	}


	void BGK_OCL::do_clear() {
		for( size_t t = 0; t < 2; t++) {
			for( size_t i = 0; i < 9; i++) {
				if( src[i]->isOnDevice() ) {
					src[i]->copyToHost();
					src[i]->deleteFromDevice();
				}

				for(size_t iy = 0; iy < gridHeight; ++iy) {
					for(size_t ix = 0; ix < gridWidth; ++ix) {
						(*(src[i]))[iy*gridWidth + ix] = fluid[i];
					}
				}

				for(size_t iy = 0; iy < gridHeight; ++iy) {
					(*(src[i]))[iy*gridWidth ] = source[i];
					(*(src[i]))[iy*gridWidth + gridWidth -1] = drain[i];
				}
				
				
				src[i]->moveToDevice();

			}
			for( size_t i = 0; i < 9; i++) {
				std::swap(src[i], dst[i]);
			}
		}

		for(size_t iy = 0; iy < gridHeight; ++iy) {
			for(size_t ix = 0; ix < gridWidth; ++ix) {
				(*flag_field)[iy*gridWidth + ix] = (int) cell_type::FLUID;
			}
		}

		for(size_t iy = 0; iy < gridHeight; ++iy) {
			(*flag_field)[iy*gridWidth] = (int) cell_type::SOURCE;
			(*flag_field)[iy*gridWidth + gridWidth-1] = (int) cell_type::SOURCE;
		}

		for(size_t ix = 0; ix < gridWidth; ++ix) {
			(*flag_field)[ ix] = (int) cell_type::NO_SLIP;
			(*flag_field)[(gridHeight-1)*gridWidth + ix] = 
				(int) cell_type::NO_SLIP;
		}


		flag_field->moveToDevice();

	}

	void BGK_OCL::do_draw(int x, int y,
						  shared_ptr<const Grid<mask_t>> mask_ptr,
						  cell_t type) {

	}

	auto BGK_OCL::get_velocity_grid() -> Grid<Vec2D<float>>* {


		if( getVelocityKernel == NULL) return NULL;

		for( size_t i = 0; i < 9; i++) {
			getVelocityKernel->input( src[i] );
		}

		getVelocityKernel->output( vel.size(), vel.data() );
		getVelocityKernel->input( (int) gridWidth );
		getVelocityKernel->input( (int) gridHeight );

		getVelocityKernel->run(2, global_size, local_size );

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
