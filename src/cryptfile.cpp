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

// Diese Datei definiert die Klasse Cryptfile

#include "cryptfile.hpp"

/**
 * @param datei verschlüsselte %Datei
 * @param schluessel Schlüssel, der beliebig groß sein kann
 */
Cryptfile::Cryptfile( Datei datei, std::vector <uint8_t> schluessel ) :
    file( std::move( datei ) ),
    key( std::move( schluessel ) )
{
    if ( ! exist() ) // Datei erstellen falls noch nicht existiert
        touch();
}

/**
 * @returns entschlüsselter Text
 */
std::string Cryptfile::decrypt() const {
    size_t index = 0;
    std::string text = file.readAll();

    for ( char& currc : text ) { // Referenz zu jedem Byte
        currc -= key[index++]; // Byte aufschlüsseln

        if ( index == key.size() ) // wenn das Ende vom Vektor erreicht ist neuanfangen
            index = 0;
    }

    return text;
}

/**
 * @param text normaler Text, der dann verschlüsselt ans Ende der %Datei geschrieben wird
 */
void Cryptfile::append( std::string text ) const {
    for ( char& currc : text ) {
        currc += key[keyat++];

        if ( keyat == key.size() )
            keyat = 0;
    }

    file.ostream( true ) << text;
}
