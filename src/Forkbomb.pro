# Copyright (C) 2016 Lukas Bartl
# Diese Datei ist Teil des Klassenchats.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Dies ist die QMake-Projekt-Datei für die Forkbome

TEMPLATE = app
DESTDIR = bin
TARGET = Chat

win32 {
    DEFINES += WIN32
    CONFIG = release windows
} else {
    CONFIG = release
    QMAKE_LFLAGS_RELEASE = -Wl,-O1,-z,relro,-z,now -s # Optimierung, RELRO und Strip
}

native {
    QMAKE_CXXFLAGS_RELEASE = -march=native
} else {
    QMAKE_CXXFLAGS_RELEASE =
}

OBJECTS_DIR = objects
MOC_DIR = $$OBJECTS_DIR
RCC_DIR = $$OBJECTS_DIR
UI_DIR = $$OBJECTS_DIR

SOURCES = forkbomb.cpp
QMAKE_CXXFLAGS = -pipe -Wall -Wextra -Wpedantic -Wdisabled-optimization -flto -ffast-math -O3 -std=c++14 -mtune=generic
QMAKE_LFLAGS_RELEASE += -flto # Link Time Optimization
