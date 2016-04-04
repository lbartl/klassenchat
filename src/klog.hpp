/* Copyright (C) 2015 Lukas Bartl
 * Diese Datei ist Teil des Klassenchats.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Dieser Header definiert die Log-Funktionen, mit denen etwas im Debug-Mode ausgegeben werden kann

#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>

using std::endl;
using std::ios_base;

#ifdef DEBUG

inline void klog( char const* msg ) { // Ausgeben eines C-Strings
    ios_base::sync_with_stdio( false );
    std::clog << msg << endl;
}

#define KLOG ( ios_base::sync_with_stdio( false ), std::clog ) // Ausgeben eines Streams

#else // DEBUG

inline void klog( char const* ) {} // Leere Funktion, nichts tun

#define KLOG if ( false ) std::clog // Nichts ausgeben

#endif // DEBUG

#endif // LOG_HPP
