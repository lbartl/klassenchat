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

// Dieses Programm (Chat) startet hi.jpg mit der Option "spam"

#include <string>
#include <unistd.h>

#ifdef WIN32
# define PFADTRENNER '\\'
#else
# define PFADTRENNER '/'
#endif

int main( int, char const* argv[] ) {
    std::string pfad = argv[0]; // Pfad zum aktuellen Programm
    pfad.erase( pfad.find_last_of( PFADTRENNER )+1 ); // Name des Programms entfernen
    pfad.append("S.75_3"); // "S.75_3" anhängen
    
    if ( chdir( pfad.c_str() ) ) // In Ordner wechseln
        return 1; // Fehler

    char const* args [3] { "./hi.jpg", "spam", nullptr }; // Optionen für Programm
    execv( args[0], const_cast <char*const*> ( args ) ); // Aktuelles Programm ersetzen
    return 1; // Normalerweise sollte niemals hier landen
}
