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

#include "DrawArrows.hpp"
#include "DrawArrowsImplementation.hpp"

using namespace std;

namespace Feldrand {

DrawArrows::DrawArrows() {
    impl = new DrawArrowsImplementation();
}

DrawArrows::~DrawArrows() {
    delete impl;
}

void DrawArrows::operator()(const Grid<Vec2D<float>>& vector_field,
                         const Grid<float>& scalar_field) {
    (*impl)(vector_field, scalar_field);
}
}
