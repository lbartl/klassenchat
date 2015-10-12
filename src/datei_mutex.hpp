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

///\file
// Dieser Header definiert das struct Datei_Mutex

#ifndef DATEI_MUTEX_HPP
#define DATEI_MUTEX_HPP

#ifdef THREAD_HPP // muss vor thread.hpp eingebunden werden, eigentlich nur für Windows, aber damit der Fehler schnell erkannt wird, auch für Unix
# error datei_mutex.hpp muss vor thread.hpp eingebunden werden!
#else

#include "datei.hpp"
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>

/// Ein file_lock, benutzbar wie eine Mutex.
/**
 * Datei_Mutex erbt von boost::interprocess::file_lock und hat nur einen Konstruktor für Datei statt C-String.
 */
struct Datei_Mutex : public boost::interprocess::file_lock {
    Datei_Mutex() = default; ///< Standard-Konstruktor.

    /// Allgemeiner Konstruktor mit Datei als Argument. Falls die %Datei nicht existiert, wird sie erstellt.
    Datei_Mutex( Datei const& file ) :
        boost::interprocess::file_lock( [&file] () { if ( ! file.exist() ) file.touch(); return file.getpath(); } () ) // Datei muss existieren
    {}

    Datei_Mutex( Datei_Mutex&& ) = default; ///< Move-Konstruktor
    Datei_Mutex& operator = ( Datei_Mutex&& ) = default; ///< Bewegender Zuweisungs-Operator
};

using file_mtx_lock = boost::interprocess::scoped_lock <Datei_Mutex>; ///< Kurzform
using sharable_file_mtx_lock = boost::interprocess::sharable_lock <Datei_Mutex>; ///< Kurzform

using namespace std::literals;

/// Undocumented.
template <typename T>
void Datei_append( Datei const& file, Datei_Mutex& file_mtx, T const& anhang ) try {
    std::ofstream os = file.ostream( true );
    file_mtx_lock f_lock ( file_mtx );
    os << anhang << std::endl;
} catch ( std::exception const& exc ) {
    Datei file1("./failbaum");
    file1.append( "Exception!\n"
                  "what(): "s + exc.what() + '\n'
                + "errno:" + strerror( errno ) );
}

/// Undocumented.
template <typename T>
void Datei_write( Datei const& file, Datei_Mutex& file_mtx, T const& neuer_inhalt ) {
    std::ofstream os = file.ostream();
    file_mtx_lock f_lock ( file_mtx );
    os << neuer_inhalt << std::endl;
}

#endif // THREAD_HPP
#endif // DATEI_MUTEX_HPP
