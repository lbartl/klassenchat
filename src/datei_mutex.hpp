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

#include "datei.hpp"
#include "thread.hpp"
#include <boost/interprocess/sync/sharable_lock.hpp>

#ifdef WIN32
# include <share.h>
class Datei_Mutex {
public:
    explicit Datei_Mutex( Datei file ) :
        lockdatei( std::move( file ) + ".lock" )
    {
        if ( ! lockdatei.exist() )
            std::ofstream( lockdatei.getpath() ); // Datei erstellen (nicht touch() wegen Exception, falls Datei gelockt ist)
    }

    Datei_Mutex( Datei_Mutex const& ) = delete;
    Datei_Mutex( Datei_Mutex&& ) = default;

    Datei_Mutex& operator = ( Datei_Mutex const& ) = delete;
    Datei_Mutex& operator = ( Datei_Mutex&& ) = default;

    bool try_lock() {
        return ( stream = _fsopen( lockdatei.getpath(), "wb", _SH_DENYRW ) );
    }

    void lock() {
        while ( ! try_lock() )
            this_thread::sleep_for( 1ms );
    }

    void unlock() {
        fclose( stream );
        stream = nullptr;
    }

    bool try_lock_sharable() {
        return ( stream = _fsopen( lockdatei.getpath(), "rb", _SH_DENYWR ) );
    }

    void lock_sharable() {
        while ( ! try_lock_sharable() )
            this_thread::sleep_for( 1ms );
    }

    void unlock_sharable() {
        unlock();
    }

private:
    FILE* stream {};
    Datei lockdatei;
};
#else
# include <boost/interprocess/sync/file_lock.hpp>
/// Ein file_lock, benutzbar wie eine Mutex.
/**
 * Datei_Mutex erbt von boost::interprocess::file_lock und hat nur einen Konstruktor für Datei statt C-String.
 */
struct Datei_Mutex : public boost::interprocess::file_lock {
    /// Allgemeiner Konstruktor mit Datei als Argument. Es wird eine neue Datei mit namen "<file>.lock" angelegt und als Lock genutzt
    explicit Datei_Mutex( Datei file ) :
        boost::interprocess::file_lock( [datei = std::move(file) + ".lock"] () { if ( ! datei.exist() ) datei.touch(); return datei.getpath(); } () ) // Datei erstellen
    {}
};
#endif

using file_mtx_lock = std::lock_guard <Datei_Mutex> const; ///< Kurzform
using sharable_file_mtx_lock = boost::interprocess::sharable_lock <Datei_Mutex>; ///< Kurzform

/// Undocumented.
inline void Datei_lock_append( Datei const& file, Datei_Mutex& file_mtx, char const*const anhang ) {
    std::ofstream os = file.ostream( true );
    file_mtx_lock f_lock ( file_mtx );
    os << anhang << std::endl;
}

/// Undocumented.
inline void Datei_lock_append( Datei const& file, Datei_Mutex& file_mtx, std::string const& anhang ) {
    Datei_lock_append( file, file_mtx, anhang.c_str() );
}

#endif // DATEI_MUTEX_HPP
