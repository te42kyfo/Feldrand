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

#include "DrawingRoutine.hpp"
#include "DrawingRoutineImplementation.hpp"

using namespace std;

namespace Feldrand {

DrawingRoutine::
DrawingRoutine() {}

DrawingRoutine::
~DrawingRoutine() {}

void
DrawingRoutine::
useColorMono() {
    impl->useColorMono();
}

void
DrawingRoutine::
useColorScalarField() {
    impl->useColorScalarField();
}

void
DrawingRoutine::
useColorVectorField() {
    impl->useColorVectorField();
}

void
DrawingRoutine::
setMonoColor(QColor color) {
    impl->setMonoColor(color);
}

QColor
DrawingRoutine::
getMonoColor() const {
    return impl->getMonoColor();
}

void
DrawingRoutine::
setMaxColor(QColor color) {
    impl->setMaxColor(color);
}

QColor
DrawingRoutine::
getMaxColor() const {
    return impl->getMaxColor();
}

void
DrawingRoutine::
setMinColor(QColor color) {
    impl->setMinColor(color);
}

QColor
DrawingRoutine::
getMinColor() const {
    return impl->getMinColor();
}

void
DrawingRoutine::
setTolerance(float tolerance) {
    impl->setTolerance(tolerance);
}

float
DrawingRoutine::
getTolerance() const {
    return impl->getTolerance();
}
}
