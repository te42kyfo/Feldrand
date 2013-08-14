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

#include "core/MRT_LBM.hpp"

using namespace std;

namespace Feldrand {

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

MRT_LBM::MRT_LBM()
    : SimulationImplementation(0.0, 0.0, 0, 0),
      src(gridWidth, gridHeight),
      dest(gridWidth, gridHeight)
{}

MRT_LBM::MRT_LBM(double width, double height, size_t grid_width, size_t grid_height)
    : SimulationImplementation(width, height, grid_width, grid_height),
      src(gridWidth, gridHeight),
      dest(gridWidth, gridHeight)
{
    do_clear();
}

MRT_LBM::MRT_LBM(MRT_LBM& other)
    : SimulationImplementation(other),
      src(gridWidth, gridHeight),
      dest(gridWidth, gridHeight)
{
    // TODO copy data
}

MRT_LBM::~MRT_LBM() {}

void MRT_LBM::one_iteration() {
    collide();
    stream();
    Grid<Cell>::swap(src, dest);

    // TODO this is a crappy NaN fix
    // I hope this can be done better
    for(size_t iy = 0; iy < src.y(); ++iy) {
        for(size_t ix = 0; ix < src.x(); ++ix) {
            bool wrong = false;
            if(!isfinite(src(ix, iy).C)) wrong = true;
            if(!isfinite(dest(ix, iy).C)) wrong = true;
            if(wrong) {
#ifndef NDEBUG
                cout << src(ix, iy);
#endif
                src(ix + 1, iy + 1) = fluid;
                src(ix + 1, iy) = fluid;
                src(ix + 1, iy - 1) = fluid;
                src(ix, iy + 1) = fluid;
                src(ix, iy) = fluid;
                src(ix, iy - 1) = fluid;
                src(ix - 1, iy + 1) = fluid;
                src(ix - 1, iy) = fluid;
                src(ix - 1, iy - 1) = fluid;
                dest(ix + 1, iy + 1) = fluid;
                dest(ix + 1, iy) = fluid;
                dest(ix + 1, iy - 1) = fluid;
                dest(ix, iy + 1) = fluid;
                dest(ix, iy) = fluid;
                dest(ix, iy - 1) = fluid;
                dest(ix - 1, iy + 1) = fluid;
                dest(ix - 1, iy) = fluid;
                dest(ix - 1, iy - 1) = fluid;
            }
        }
    }
}

void MRT_LBM::do_clear() {
    for(size_t iy = 0; iy < src.y(); ++iy) {
        for(size_t ix = 0; ix < src.x(); ++ix) {
            src(ix, iy) = fluid;
            dest(ix, iy) = fluid;
        }
    }
    // TODO remove this fun hack that makes clear initialize a wind tunnel
    for(size_t iy = 0; iy < src.y(); ++iy) {
        size_t ix = 0;
        src(ix, iy) = source;
        dest(ix, iy) = source;
    }
    for(size_t iy = 0; iy < src.y(); ++iy) {
        size_t ix = gridWidth - 1;
        src(ix, iy) = drain;
        dest(ix, iy) = drain;
    }
}

void MRT_LBM::do_draw(int x, int y,
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
auto MRT_LBM::get_velocity_grid() -> Grid<Vec2D<float>>* {
    Grid<Vec2D<float>>* g(new Grid<Vec2D<float>>(gridWidth, gridHeight));
    for(size_t iy = 0; iy < src.y(); ++iy) {
        for(size_t ix = 0; ix < src.x(); ++ix) {
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

auto MRT_LBM::get_density_grid()  -> Grid<float>* {
    Grid<float>* g(new Grid<float>(gridWidth, gridHeight));
    for(size_t iy = 0; iy < gridHeight; ++iy) {
        for(size_t ix = 0; ix < gridWidth; ++ix) {
            const Cell& cell = src(ix, iy);
            float d =    cell.NW +       cell.N +       cell.NE
                      + cell.W  +       cell.C +       cell.E
                      + cell.SW +       cell.S +       cell.SE;
            (*g)(ix, iy) = d;
        }
    }
    return g;
}

auto MRT_LBM::get_type_grid()     -> Grid<cell_t>* {
    Grid<cell_t>* g(new Grid<cell_t>(gridWidth, gridHeight));
    for(size_t iy = 0; iy < gridHeight; ++iy) {
        for(size_t ix = 0; ix < gridWidth; ++ix) {
            const Cell& cell = src(ix, iy);
            (*g)(ix, iy) = cell.type;
        }
    }
    return g;
}

void MRT_LBM::write_data(std::ostream& dest) {
    dest << this->src;
    dest << this->dest;
}

void MRT_LBM::read_data(std::istream& src) {
    src >> this->src;
    src >> this->dest;
}

/* The streaming step of the MRT-LBM simulation. The values of each fluid
 * cell are exchanged with their neighbors or reflected should the
 * neighbor be an obstacle cell */
void MRT_LBM::stream() {
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
void MRT_LBM::collide() {
    float omega = 1.6;
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

            m2 =        cell.NW - 2.0 * cell.N +       cell.NE
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
            if(cell.NW  < 0.0) cell.NW  = 0.0;
            if(cell.N   < 0.0) cell.N   = 0.0;
            if(cell.NE  < 0.0) cell.NE  = 0.0;
            if(cell.W   < 0.0) cell.W   = 0.0;
            if(cell.C   < 0.0) cell.C   = 0.0;
            if(cell.E   < 0.0) cell.E   = 0.0;
            if(cell.SW  < 0.0) cell.SW  = 0.0;
            if(cell.S   < 0.0) cell.S   = 0.0;
            if(cell.SE  < 0.0) cell.SE  = 0.0;

            /* stability workaround */
            if(cell.NW > 10.0e5) cell.NW  /= 2.0;
            if(cell.N  > 10.0e5) cell.N   /= 2.0;
            if(cell.NE > 10.0e5) cell.NE  /= 2.0;
            if(cell.W  > 10.0e5) cell.W   /= 2.0;
            if(cell.C  > 10.0e5) cell.C   /= 2.0;
            if(cell.E  > 10.0e5) cell.E   /= 2.0;
            if(cell.SW > 10.0e5) cell.SW  /= 2.0;
            if(cell.S  > 10.0e5) cell.S   /= 2.0;
            if(cell.SE > 10.0e5) cell.SE  /= 2.0;
        }
    }
}
}
