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

#ifndef FELDRAND__BGK_OCL_HPP
#define FELDRAND__BGK_OCL_HPP

#include "core/SimulationImplementation.hpp"
#include "OpenClHelper/OpenCLHelper.h"
#include "OpenClHelper/CLKernel.h"


namespace Feldrand {


	class BGK_OCL : public Simulation::SimulationImplementation {
	public:
		BGK_OCL();
        BGK_OCL(std::string filename);
		BGK_OCL(double width, double height,
				size_t grid_width, size_t grid_height);
		BGK_OCL(BGK_OCL& other);
		virtual ~BGK_OCL();
	protected:
		void init();
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
		
	private:
		void setFields(const size_t ix, const size_t iy, 
					   const float* val, const int type);

		CLKernel* getVelocityKernel;
		CLKernel* getDensityKernel;
		CLKernel* simulationStepKernel;
		OpenCLHelper* cl;
		CLArrayFloat* dst[9];
		CLArrayFloat* src[9];
		CLArrayInt* flag_field;

		size_t global_size[2];
		size_t local_size[2];

        std::vector<unsigned char> image;
      
		std::vector<float> vel;
		std::vector<float> density;
	};
}
#endif // FELDRAND__BGK_OCL_HPP
