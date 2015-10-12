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

// Diese Datei definiert die Memberfunktion Datei::readAll() und die Konstanten von filesystem.hpp

#include "filesystem.hpp"
#include <iostream>

/**
 * @returns Inhalt der %Datei.
 */
std::string Datei::readAll() const try {
    std::ifstream stream ( file, std::ios::ate | std::ios::binary ); // mit ate am Ende starten
    stream.exceptions( std::ios::badbit | std::ios::failbit );

    size_t const length = stream.tellg(); // Länge des Streams
    stream.seekg( 0 ); // An Anfang springen

    std::string text( length, '\0' ); // Speicher anlegen
    stream.read( &text[0], length ); // stream in text schreiben

    return text;
} catch ( fstream_exc const& exc ) {
    ifstreamExcAusgabe( exc, *this );
    return ""; // leerer String für die meisten Fälle gut
}

namespace static_paths {
    extern Ordner const terminatedir = "./wichtig",  ///< Ordner, in dem eine %Datei mit Benutzernamen liegt, falls der %Admin ihn rausgeschmissen hat
                        senddir      = "./personal", ///< Ordner, in dem Privatchats gespeichert sind
                        infodir      = "./infos";    ///< Ordner, in dem Infos an Benutzernamen gespeichert sind

    extern Datei const alltfile      = "./all-terminate", ///< Datei, die, wenn sie vorhanden ist, anzeigt, dass überall der %Chat geschlossen werden soll
                       warnfile      = "./warning",       ///< Datei, die, wenn sie vorhanden ist, anzeigt, dass überall eine %Warnung erscheinen soll
                       nutzerfile    = "./nutzer",        ///< Datei, in der alle angemeldeten Benutzernamen gespeichert sind
                       adminfile     = "./boss",          ///< Datei, in der alle angemeldeten Admins gespeichert sind
                       verbotenfile  = "./forbid",        ///< Datei, in der alle verbotenen Benuternamen gespeichert sind
                       passfile      = "./pass.jpg";      ///< Datei, in der alle Passwörter der std_admins gespeichert sind
}
