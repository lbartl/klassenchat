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
// Dieser Header deklariert und definiert die Klasse Ordner und die Funktion chDir(), mit der Ordner verwaltet werden können und definiert Funktionen zum Ausgeben von Datei-Exceptions

#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include "datei.hpp"
#include <iostream>

/// Mit der Klasse Ordner können Verzeichnisse verwaltet werden.
/**
 * Ordner erbt von boost::filesystem::path und definiert nur ein paar zusätzliche Methoden speziell für Verzeichnisse.
 */
struct Ordner : public fs::path
{
    /// Konstruktor mit Pfad als C-String.
    Ordner( char const*const pfad = "" ) :
        fs::path( pfad ) {}

    /// Konstruktor mit Pfad als boost::filesystem::path.
    Ordner( fs::path const& pfad ) :
        fs::path( pfad ) {}

    /// Expliziter Konstruktor mit Pfad als std::string.
    explicit Ordner( std::string const& pfad ) :
        fs::path( pfad ) {}

    /// Pfad als C-String zurückgeben.
    char const* getpath() const noexcept {
        return string().c_str();
    }

    /// %Ordner entfernen.
    void remove() const {
        fs::remove_all( *this );
    }

    /// %Ordner erstellen.
    void create() const {
        fs::create_directories( *this );
    }

    /// Alle Dateien des Ordners entfernen.
    void removeInhalt() const {
        remove();
        create();
    }
};

/// Aktuelles Verzeichnis wechseln.
/**
 * @param ordner In diesen %Ordner wird gewechselt
 */
inline void chDir( Ordner const& ordner ) { // In ordner wechseln
    fs::current_path( ordner );
}

/// Statische Dateien und Ordner.
namespace static_paths { // definiert in definitions.cpp
    extern Ordner const terminatedir, senddir, infodir;
    extern Datei const alltfile, warnfile, nutzerfile, adminfile, verbotenfile, passfile;
}

/// Erstellt eine Datei für einen %Nutzer.
/**
 * @param folder der Ordner, in dem die Datei erstellt wird
 * @param plum ob der %Nutzer im Plum-Chat ist
 * @param benutzername der Benutzername
 * @returns Datei für %Nutzer.
 *
 * Erstellt eine Datei in folder, die als Namen den Benutzernamen und danach "_1" für Plum-Chat und "_0" für normalen %Chat hat.
 */
inline Datei makeToNutzerDatei( Ordner const& folder, bool const plum, std::string const& benutzername ) {
    return folder / benutzername + ( plum ? "_1" : "_0" );
}

/// Ruft \ref makeToNutzerDatei auf. Sinnvoll in Kombination mit fromBenutzername_str()
inline Datei makeToNutzerDatei( Ordner const& folder, std::pair <bool, std::string> const& paar ) {
    return makeToNutzerDatei( folder, paar.first, paar.second );
}

// Exceptions

/// Gibt Informationen zu einem fstream-Fehler aus.
/**
 * @param exc fstream-Exception
 * @param msg auszugebende Nachricht
 *
 * Es werden msg zusammen mit einem Ausrufezeichen ausgegeben,
 * danach wird exc.what() ausgegeben, und dann der Error-Code und die Bedeutung des Error-Codes.
 */
inline void fstreamExcAusgabe( fstream_exc const& exc, char const*const msg ) { // Information zu einer fstream_exc ausgeben
    std::cerr << '\n' << msg << "!\n"
                 "what(): " << exc.what() << "\n"
                 "Error-Code: " << errno << "\n"
                 "Bedeutung: " << strerror( errno ) << '\n';
#ifdef _DEBUG
    std::cerr << '\n';
#endif
}

/// Kurzform für \ref fstreamExcAusgabe( exc, msg.c_str() )
inline void fstreamExcAusgabe( fstream_exc const& exc, std::string const& msg ) {
    fstreamExcAusgabe( exc, msg.c_str() );
}

/// Gibt Informationen zu einem ofstream-Fehler aus.
/**
 * Ruft \ref fstreamExcAusgabe auf mit der Nachricht "Fehler beim Schreiben in Datei <file>".
 */
inline void ofstreamExcAusgabe( fstream_exc const& exc, Datei const& file ) {
    fstreamExcAusgabe( exc, "Fehler beim Schreiben in Datei " + file );
}

/// Gibt Informationen zu einem ifstream-Fehler aus.
/**
 * Ruft \ref fstreamExcAusgabe auf mit der Nachricht "Fehler beim Lesen der Datei <file>".
 */
inline void ifstreamExcAusgabe( fstream_exc const& exc, Datei const& file ) {
    fstreamExcAusgabe( exc, "Fehler beim Lesen der Datei " + file );
}

#endif // FILESYSTEM_HPP
