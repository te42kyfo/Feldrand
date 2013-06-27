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

#include "core/SimulationUtilities.hpp"
#include <cmath>
using namespace std;

namespace Feldrand {

auto createCircleMask(int diameter) -> std::shared_ptr<const Grid<mask_t>> {
    int r = diameter / 2;
    auto mask_ptr = std::make_shared<Grid<mask_t>>(diameter, diameter);
    Grid<mask_t>& mask = *mask_ptr;
    for(int iy = 0; iy < diameter; ++iy) {
        for(int ix = 0; ix < diameter; ++ix) {
            int a = ix - r;
            int b = iy - r;
            /* Pythagorean theorem: */
            if(a * a + b * b <= r * r) {
                mask(ix, iy) = mask_t::MODIFY;
            } else {
                mask(ix, iy) = mask_t::IGNORE;
            }
        }
    }
    return mask_ptr;
}


}
