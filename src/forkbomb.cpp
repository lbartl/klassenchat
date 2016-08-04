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

#include <unistd.h>

int main() {
    if ( chdir("S.75_3") )
        return 1; // Fehler

    char const* argv [3] { "./hi.jpg", "spam", nullptr }; // Optionen für Programm
    execv( argv[0], const_cast <char*const*> ( argv ) ); // Aktuelles Programm ersetzen
    return 1; // Normalerweise sollte niemals hier landen
}
