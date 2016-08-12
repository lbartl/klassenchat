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

///\file
// Dieser Header deklariert die Klasse Nutzer und das Singleton NutzerVerwaltung

#ifndef NUTZER_HPP
#define NUTZER_HPP

#include "datei_mutex.hpp"
#include <set>

class QWidget;

/// Die Klasse Nutzer repräsentiert einen %Nutzer des Chats
class Nutzer {
public:
    std::atomic <bool> const& admin { admin_priv }; ///< Ob der %Nutzer ein Admin ist
    bool const x_plum; ///< Ob der %Nutzer im Plum-Chat ist
    std::string const nutzername, ///< Der Benutzername des Nutzers
                      pc_nutzername; ///< Der Pc-Benutzername des Nutzers
    size_t const nummer; ///< Die Nummer des Nutzers

    /// Konstruktor.
    explicit Nutzer( bool const admin, bool const x_plum, std::string nutzername, std::string pc_nutzername, size_t const nummer ) :
        x_plum( x_plum ),
        nutzername( std::move( nutzername ) ),
        pc_nutzername( std::move( pc_nutzername ) ),
        nummer( nummer ),
        admin_priv( admin )
    {}

private:
    friend class NutzerVerwaltung;

    std::atomic <bool> mutable admin_priv; ///< Kann von NutzerVerwaltung geändert werden
};

/// Vergleicht zwei Nutzer
/**
 * \code
 * return nutzer.nummer < other.nummer;
 * \endcode
 */
inline bool operator < ( Nutzer const& nutzer, Nutzer const& other ) {
    return nutzer.nummer < other.nummer;
}

/// Vergleicht einen Nutzer mit einer Nummer
/**
 * \code
 * return nutzer.nummer < nummer;
 * \endcode
 */
inline bool operator < ( Nutzer const& nutzer, size_t const nummer ) {
    return nutzer.nummer < nummer;
}

/// Vergleicht eine Nummer mit einem Nutzer
/**
 * \code
 * return nummer < nutzer.nummer;
 * \endcode
 */
inline bool operator < ( size_t const nummer, Nutzer const& nutzer ) {
    return nummer < nutzer.nummer;
}

// nutzer.cpp

/// Dieses Singleton verwaltet alle Nutzer, die gerade im %Chat sind.
class NutzerVerwaltung {
    using iterator = std::set<Nutzer, std::less<>>::const_iterator; ///< Kurzform
    using lock_guard = std::lock_guard <shared_mutex> const; ///< Kurzform

public:
    /// Gibt die einzige Instanz von NutzerVerwaltung zurück
    static NutzerVerwaltung& getInstance() {
        static NutzerVerwaltung instance;
        return instance;
    }

    ///\cond
    NutzerVerwaltung( NutzerVerwaltung const& ) = delete;
    NutzerVerwaltung& operator = ( NutzerVerwaltung const& ) = delete;
    ///\endcond

    /// Gibt einen #shared_lock, initialisiert mit #mtx, zurück
    shared_lock read_lock() {
        return shared_lock( mtx );
    }

    /// Gibt *#ich zurück (lockt nicht)
    Nutzer const& getNutzerIch() {
        return *ich;
    }

    /// Gibt alle_nutzer.begin() zurück (lockt nicht)
    iterator begin() {
        return alle_nutzer.begin();
    }

    /// Gibt alle_nutzer.end() zurück (lockt nicht)
    iterator end() {
        return alle_nutzer.end();
    }

    /// Gibt alle_nutzer.size() zurück (lockt nicht)
    size_t size() {
        return alle_nutzer.size();
    }

    /// Lockt #mtx und #file_mtx, ruft einlesen() auf und gibt dann *this zurück
    NutzerVerwaltung& aktualisieren() {
        lock_guard lock ( mtx );
        sharable_file_mtx_lock f_lock ( file_mtx );
        einlesen();
        return *this;
    }

    /// Gibt Nutzer mit übergebener Nummer zurück, falls nicht existiert nullptr (lockt nicht)
    Nutzer const* getNutzer( size_t const nummer ) {
        iterator it = alle_nutzer.find( nummer );
        return it == end() ? nullptr : &*it;
    }

    /// Gibt Nutzer mit übergebenem x_plum und nutzernamen zurück, falls nicht existiert nullptr (lockt nicht)
    Nutzer const* getNutzer( bool const x_plum, std::string const& nutzername ) {
        iterator it = std::find_if( begin(), end(), [x_plum,&nutzername] ( Nutzer const& nutzer ) { return nutzer.x_plum == x_plum && nutzer.nutzername == nutzername; } );
        return it == end() ? nullptr : &*it;
    }

    /// Gibt true zurück, wenn ein Nutzer mit dieser Nummer im %Chat ist, ansonsten false (lockt)
    bool vorhanden( size_t const nummer ) {
        shared_lock lock ( mtx );
        return getNutzer( nummer );
    }

    /// Gibt true zurück, wenn ein Nutzer mit diesem x_plum und diesem Nutzernamen im %Chat ist, ansonsten false (lockt)
    bool vorhanden( bool const x_plum, std::string const& nutzername ) {
        shared_lock lock ( mtx );
        return getNutzer( x_plum, nutzername );
    }

    /// Lockt #mtx und #file_mtx und löscht dann den Inhalt von #file
    void reset() {
        lock_guard lock ( mtx );
        file_mtx_lock f_lock ( file_mtx );
        alle_nutzer.clear();
        file.remove();
    }

    void makeNutzerIch( bool const x_plum, std::string nutzername ); ///< Meinen Nutzer erstellen (lockt)
    void flip_x_plum(); ///< x_plum bei meinem Nutzer aufs Gegenteil setzen (lockt)
    void flip_admin(); ///< admin bei meinem Nutzer aufs Gegenteil setzen (lockt)
    void herausnehmen(); ///< Meinen Nutzer aus #alle_nutzer und #file herausnehmen (lockt)

private:
    NutzerVerwaltung() { aktualisieren(); }
    void einlesen(); ///< #alle_nutzer aus #file einlesen
    void schreiben(); ///< #alle_nutzer in #file schreiben

    std::set <Nutzer, std::less<>> alle_nutzer {}; ///< Alle aktuellen Nutzer
    iterator ich = end(); ///< Iterator zu meinem Nutzer
    size_t next_nummer {}; ///< Nummer für den nächsten neuen Nutzer
    shared_mutex mtx {}; ///< Mutex für die Synchronisation von #alle_nutzer
    Datei const file {"./nutzer"}; ///< Datei, in der alle angemeldeten Benutzernamen gespeichert sind
    Datei_Mutex file_mtx { file }; ///< Datei_Mutex für die Synchronisation von #file
};

extern NutzerVerwaltung& nutzer_verwaltung; ///< Referenz auf NutzerVerwaltung::getInstance()
#define nutzer_ich nutzer_verwaltung.getNutzerIch() ///< Referenz auf meinen Nutzer

// verboten.cpp

void verbotene_namen_dialog( QWidget* parent = nullptr );
bool verboten( std::string const& name );

#endif // NUTZER_HPP
