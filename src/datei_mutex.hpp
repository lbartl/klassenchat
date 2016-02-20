/* Copyright (C) 2015,2016 Lukas Bartl
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

#ifdef THREAD_HPP // muss vor thread.hpp eingebunden werden, eigentlich nur für Windows, aber, damit der Fehler schnell erkannt wird, auch für Unix
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

    /// Allgemeiner Konstruktor mit Datei als Argument. Es wird eine neue Datei mit namen "<file>.lock" angelegt und als Lock genutzt
    Datei_Mutex( Datei const& file ) :
        boost::interprocess::file_lock( [datei = file + ".lock"] () { if ( ! datei.exist() ) datei.touch(); return datei.getpath(); } () ) // Datei erstellen
    {}

    Datei_Mutex( Datei_Mutex&& ) = default; ///< Move-Konstruktor
    Datei_Mutex& operator = ( Datei_Mutex&& ) = default; ///< Bewegender Zuweisungs-Operator
};

using file_mtx_lock = boost::interprocess::scoped_lock <Datei_Mutex>; ///< Kurzform
using sharable_file_mtx_lock = boost::interprocess::sharable_lock <Datei_Mutex>; ///< Kurzform

using namespace std::literals;

/// Undocumented.
inline void Datei_lock_append( Datei const& file, Datei_Mutex& file_mtx, std::string const& anhang ) {
    Datei_lock_append( file, file_mtx, anhang.c_str() );
}

/// Undocumented.
inline void Datei_lock_append( Datei const& file, Datei_Mutex& file_mtx, char const*const anhang ) {
    std::ofstream os = file.ostream( true );
    file_mtx_lock f_lock ( file_mtx );
    os << anhang << std::endl;
}

#endif // THREAD_HPP
#endif // DATEI_MUTEX_HPP
