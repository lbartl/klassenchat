/* Copyright (C) 2016 Lukas Bartl
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

// Dieser Header deklariert die Klasse Nutzer und das Singleton NutzerVerwaltung

#ifndef NUTZER_HPP
#define NUTZER_HPP

///\cond

#include "hineinschreiben.hpp"
#include <set>
#include <shared_mutex>

#ifdef WIN32
# include <boost/thread/shared_mutex.hpp>
using boost::shared_mutex;
#else
using shared_mutex = std::shared_timed_mutex;
#endif

using shared_lock = std::shared_lock <shared_mutex>;

struct Nutzer {
    bool const x_plum, admin;
    std::string const nutzername, pc_nutzername;
    size_t const nummer;

    explicit Nutzer( bool const x_plum, bool const admin, std::string nutzername, std::string pc_nutzername, size_t const nummer ) :
        x_plum( x_plum ),
        admin( admin ),
        nutzername( std::move( nutzername ) ),
        pc_nutzername( std::move( pc_nutzername ) ),
        nummer( nummer )
    {}

    bool operator < ( Nutzer const& other ) const {
        return nummer < other.nummer;
    }
};

inline bool operator < ( Nutzer const& nutzer, size_t const nummer ) {
    return nutzer.nummer < nummer;
}

inline bool operator < ( size_t const nummer, Nutzer const& nutzer ) {
    return nummer < nutzer.nummer;
}

// nutzer.cpp

class NutzerVerwaltung {
    using iterator = std::set<Nutzer, std::less<>>::const_iterator;
    using lock_guard = std::lock_guard <shared_mutex> const;

public:
    static NutzerVerwaltung& getInstance() {
        static NutzerVerwaltung instance;
        return instance;
    }

    NutzerVerwaltung( NutzerVerwaltung const& ) = delete;
    NutzerVerwaltung& operator = ( NutzerVerwaltung const& ) = delete;

    shared_lock read_lock() {
        return shared_lock( mtx );
    }

    Nutzer const& getNutzerIch() {
        return *ich;
    }

    iterator begin() {
        return alle_nutzer.begin();
    }

    iterator end() {
        return alle_nutzer.end();
    }

    size_t size() {
        return alle_nutzer.size();
    }

    NutzerVerwaltung& aktualisieren() {
        lock_guard lock ( mtx );
        sharable_file_mtx_lock f_lock ( file_mtx );
        einlesen();
        return *this;
    }

    Nutzer const* getNutzer( size_t const nummer ) {
        shared_lock lock ( mtx );
        iterator it = alle_nutzer.find( nummer );
        return it == end() ? nullptr : &*it;
    }

    Nutzer const* getNutzer( bool const x_plum, std::string const& nutzername ) {
        shared_lock lock ( mtx );
        iterator it = std::find_if( begin(), end(), [x_plum,&nutzername] ( Nutzer const& nutzer ) { return nutzer.x_plum == x_plum && nutzer.nutzername == nutzername; } );
        return it == end() ? nullptr : &*it;
    }

    void reset() {
        lock_guard lock ( mtx );
        file_mtx_lock f_lock ( file_mtx );
        alle_nutzer.clear();
        file.remove();
    }

    void makeNutzerIch( bool const x_plum, std::string nutzername );
    void flip_x_plum();
    void flip_admin();
    void herausnehmen();

private:
    NutzerVerwaltung() { aktualisieren(); }
    void einlesen();
    void schreiben();

    std::set <Nutzer, std::less<>> alle_nutzer {};
    iterator ich = end();
    size_t next_nummer {}; ///< Nummer für den nächsten neuen Nutzer
    shared_mutex mtx {};
    Datei const file {"./nutzer"}; ///< Datei, in der alle angemeldeten Benutzernamen gespeichert sind
    Datei_Mutex file_mtx { file };
};

extern NutzerVerwaltung& nutzer_verwaltung;
#define nutzer_ich nutzer_verwaltung.getNutzerIch()

// verboten.cpp

class QWidget;

void verbotene_namen_dialog( QWidget* parent = nullptr );
bool verboten( std::string const& name );

///\endcond
#endif // NUTZER_HPP
