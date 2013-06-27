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

#include <QApplication>
#include <cstdlib>
#include <iostream>
#include "config.hpp"
#include "MainWindow.hpp"

using namespace std;

void parseArgs(int argc, char* argv[]);

int main(int argc, char* argv[]) {

    QApplication app(argc, argv);
    parseArgs(argc, argv);

    Feldrand::MainWindow window;
    window.resize(window.sizeHint());

    window.show();

    return app.exec();
}

void parseArgs(int argc, char* argv[]) {
    switch(argc) {
    case 1:
        return;
    case 2:
        if(0 == strncmp(argv[1], "--version", 9)) {
            cout << "FELDRAND " << Feldrand::version << endl;
            exit(EXIT_SUCCESS);
        }
    default:
        cout << "FELDRAND\n";
        cout << "usage: " << argv[0] << " [OPTIONS]\n";
        cout << "valid options are:\n";
        cout << "  --version   show the feldrand version" << endl;
        exit(EXIT_SUCCESS);
    }
}
