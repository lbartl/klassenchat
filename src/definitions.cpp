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

// Diese Datei definiert die Memberfunktion Datei::readAll(), die Konstanten von filesystem.hpp und die globalen Variablen von global.hpp

#include "filesystem.hpp"

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
