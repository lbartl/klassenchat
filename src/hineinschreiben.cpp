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

// Diese Datei definiert die Klasse Hineinschreiben

#include "hineinschreiben.hpp"
#include "thread.hpp"
#include "filesystem.hpp"
#include "klog.hpp"

using std::string;

/**
 * @param benutzername_str mein Benutzername_str
 * @param datei die %Datei, in der die Benutzernamen stehen.
 * \b Wichtig: Hineinschreiben speichert nur eine Referenz auf dieses Datei-Objekt, deswegen muss es länger als das Hineinschreiben-Objekt existieren!
 *
 * Um Methoden wie reinschreiben() und hineingeschrieben() zu nutzen, muss dieser Konstruktor benutzt werden,
 * da sonst mein Benutzername_str nicht bekannt ist.
 */
Hineinschreiben::Hineinschreiben( QString const& benutzername_str, Datei const& datei ) :
    nutzername_str( benutzername_str ),
    file( std::move( datei ) ),
    namen(),
    file_mtx( file )
{
    aktualisieren();
}

/**
 * @param datei die %Datei, in der die Benutzernamen stehen
 *
 * Dieser Konstruktor kann benutzt werden, wenn nur die vorhanden Benutzernamen wichtig sind
 * und Methoden wie reinschreiben() und hineingeschrieben() nicht benötigt werden.
 */
Hineinschreiben::Hineinschreiben( Datei const& datei ) :
    Hineinschreiben( "$$$", std::move( datei ) ) {}

/**
 * @returns *this.
 *
 * Die %Datei neu einlesen und alle Benutzernamen in den Arbeitsspeicher laden.
 */
Hineinschreiben& Hineinschreiben::aktualisieren() {
    constexpr size_t max_length = 20*( sizeof("Ü") - 1 ) + sizeof(" (Plum-Chat)"); // UTF-8, deswegen sind Umlaute größer als ein Byte
    static char name [max_length]; // Benutzername_str des aktuellen Nutzers
    static std::mutex mut;

    sharable_file_mtx_lock lock ( file_mtx );
    std::ifstream datei = file.istream();
    namen.clear();

    while ( true ) {
        lock_guard lock ( mut ); // Nötig, da name static ist, und somit nicht gleichzeitig genutzt werden darf
        if ( ! datei.getline( name, max_length ) ) break;
        namen.emplace_back( name );
    }

    return *this;
}

void Hineinschreiben::reinschreiben() {
    if ( nutzername_str == "$$$" )
        throw std::logic_error("Keinen Nutzernamen gesetzt!");

    Datei_append( file, file_mtx, nutzername_str.toStdString() );
    namen.push_back( nutzername_str );
}

void Hineinschreiben::herausnehmen() {
    if ( nutzername_str == "$$$" )
        throw std::logic_error("Keinen Nutzernamen gesetzt!");

    aktualisieren();

    std::ofstream datei = file.ostream();
    file_mtx_lock lock ( file_mtx );

    for ( QString const& currnutzer : namen ) // Alle anderen Benutzername_str wieder in Datei schreiben
        if ( currnutzer != nutzername_str )
            datei << currnutzer.toStdString() << '\n';

    datei.flush();
}

/**
 * Alle Benutzernamen aus meinem %Chat (also normaler %Chat oder Plum-Chat) zurückgeben.
 * Bei Nutzern aus dem Plum-Chat wird das " (Plum-Chat)" Anhängsel entfernt.
 */
std::vector <string> Hineinschreiben::namen_meinchat() const {
    if ( nutzername_str == "$$$" )
        throw std::logic_error("Nicht gesetzt ob im Plum-Chat oder nicht!");

    bool const x_plum = nutzername_str.contains('('); // Ob ich im Plum-Chat bin
    std::vector <string> meinchat;
    bool plum;
    string benutzername;

    for ( QString const& currnutzer : namen ) {
        std::tie( plum, benutzername ) = fromBenutzername_str( currnutzer );

        if ( plum == x_plum ) // Gleicher Chat
            meinchat.push_back( std::move( benutzername ) );
    }

    return meinchat;
}
