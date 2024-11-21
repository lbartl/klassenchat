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
// Dieser Header deklariert und definiert die Klasse Ordner und die Funktion chDir(), mit der Ordner verwaltet werden können und definiert Funktionen zum Ausgeben von Datei-Exceptions

#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include "nutzer.hpp"
#include <iostream>

/// Mit der Klasse Ordner können Verzeichnisse verwaltet werden.
/**
 * Ordner erbt von boost::filesystem::path und definiert nur ein paar zusätzliche Methoden speziell für Verzeichnisse.
 */
struct Ordner : public fs::path {
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

/// NutzerDateiOstream erstellt einen std::ofstream für eine Nutzer-Datei (siehe makeToNutzerDatei())
class NutzerDateiOstream : public std::ofstream {
public:
    /// Konstruktor, erstellt #datei.
    /**
     * @param ordner der Ordner, in dem die Datei erstellt wird
     * @param nummer Nummer des Nutzers
     */
    explicit NutzerDateiOstream( Ordner const& ordner, size_t const nummer ) :
        std::ofstream(),
        datei( ordner / std::to_string( nummer ) )
    {
        for ( size_t i = 0; datei.exist() && i < 50; ++i ) // Nach 5s ist der Nutzer wohl nicht mehr im Chat
            this_thread::sleep_for( 100ms );

        this->open( datei.getpath(), binary );
        this->exceptions( badbit | failbit );
    }

    /// Konstruktor mit Nutzer statt Nummer.
    explicit NutzerDateiOstream( Ordner const& ordner, Nutzer const& nutzer ) :
        NutzerDateiOstream( ordner, nutzer.nummer )
    {}

    /// Move-Konstruktor.
    NutzerDateiOstream( NutzerDateiOstream&& other ) :
        std::ofstream( std::move( other ) ),
        datei( std::move( other.datei ) )
    {}

    /// Destruktor. Ruft close() auf.
    ~NutzerDateiOstream() {
        close();
    }

    /// Schließt das schreiben ab und benennt #datei in die eigentliche Nutzer-Datei um.
    void close() {
        Datei const nutzerDatei ( datei.getpath() + ".jpg"s );

        for ( size_t i = 0; nutzerDatei.exist() && i < 50; ++i ) // Nach 5s ist der Nutzer wohl nicht mehr im Chat
            this_thread::sleep_for( 100ms );

        std::ofstream::close(); // Schreiben abschließen
        fs::rename( datei.getpath(), nutzerDatei.getpath() );
    }

private:
    Datei datei; ///< Die Zwischen-Datei, in diese wird geschrieben
};

/// Statische Dateien und %Ordner.
namespace static_paths {
    inline Ordner const terminatedir = "./wichtig",  ///< Ordner, in dem Dateien zum %Entfernen von Nutzern gespeichert sind
                        senddir      = "./personal", ///< Ordner, in dem Privatchats gespeichert sind
                        infodir      = "./infos",    ///< Ordner, in dem Infos an %Nutzer gespeichert sind
                        checkdir     =  "./check";   ///< Ordner, in dem Dateien zum Überprüfen, ob ein %Nutzer noch im %Chat ist, sind

    inline Datei const alltfile      = "./all-terminate", ///< Datei, die, wenn sie vorhanden ist, anzeigt, dass überall der %Chat geschlossen werden soll
                       warnfile      = "./warning";       ///< Datei, die, wenn sie vorhanden ist, anzeigt, dass überall eine %Warnung erscheinen soll
}

/// Erstellt eine Datei für einen Nutzer.
/**
 * @param folder der Ordner, in dem die Datei erstellt wird
 * @param nummer Nummer des Nutzers
 * @returns Datei für Nutzer.
 *
 * Erstellt eine Datei in \a folder, die als Namen die Nummer des Nutzers hat mit der Dateiendung ".jpg"
 */
inline Datei makeToNutzerDatei( Ordner const& folder, size_t const nummer ) {
    return folder / std::to_string( nummer ) + ".jpg";
}

/// Erstellt eine Datei für einen Nutzer.
/**
 * @param folder der Ordner, in dem die Datei erstellt wird
 * @param nutzer der Nutzer
 * @returns Datei für Nutzer.
 *
 * Erstellt eine Datei in \a folder, die als Namen die Nummer des Nutzers hat mit der Dateiendung ".jpg"
 */
inline Datei makeToNutzerDatei( Ordner const& folder, Nutzer const& nutzer ) {
    return makeToNutzerDatei( folder, nutzer.nummer );
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
#ifdef DEBUG
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
