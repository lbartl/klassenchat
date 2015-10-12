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

// Diese Datei definiert die Klasse AdminPass

#include "adminpass.hpp"
#include "filesystem.hpp"
#include "klog.hpp"

using std::string;

AdminPass::AdminPass( std::string const& benutzername ) :
    nutzername( benutzername ),
    passfile( static_paths::passfile, { 75, 45, 114, 30, 6, 203, 13, 102, 77, 155, 97, 100, 201, 170, 209, 178, 67, 102, 18, 218, 94, 106, 159, 126, 54, 65, 151, 91, 129, 107, 125, 102, 215, 96, 169, 18 } ),
    file_mtx( static_paths::passfile )
{
    fromFile();
}

void AdminPass::fromFile() { // Passwörter aktualisieren
    allpass.clear();

    file_mtx.lock_sharable();
        string const text = passfile.decrypt();
    file_mtx.unlock_sharable();

    char const *str_pos = text.c_str(), *name_end, *line_end;

    while (( name_end = strchr( str_pos, '#' ) )) {
        line_end = strchr( name_end, '\n' );
        allpass.emplace( string( str_pos, name_end ), string( name_end + 1, line_end ) ); // für jeden Benutzernamen das Passwort speichern
        str_pos = line_end + 1;
    }
}

void AdminPass::toFile() { // Passwörter schreiben
    string new_inhalt;

    for ( auto const& currp : allpass )
        new_inhalt += currp.first + '#' + currp.second + '\n';

    file_mtx_lock lock ( file_mtx );
    passfile.encrypt( std::move( new_inhalt ) );
}

/**
 * @param benutzername der Benutzername
 * @returns %Passwort des Benutzernamens
 *
 * Wenn der Benutzername nicht gefunden wird, wird #std_pass zurückgegeben.
 * Es wird \b nicht überprüft, ob Benutzername in #Chat::std_admins vorhanden ist.
 */
string AdminPass::getpass( string const& benutzername ) const {
    auto pass_it = allpass.find( benutzername ); // Iterator zum Passwort von benutzername

    return pass_it == allpass.end() ? std_pass : pass_it -> second; // Passwort oder Standard-Passwort zurückgeben
}

/**
 * @param newpass neues Passwort (unverschlüsselt)
 *
 * Wirft std::invalid_argument, falls das newpass leer ist oder ungültige Zeichen (Newline und Null-Byte) enthält.
 */
void AdminPass::setpass( string newpass ) {
    if ( newpass == "" )
        throw std::invalid_argument("Kein Passwort eingegeben!");
    else if ( newpass.find('\n') != newpass.npos || newpass.find('\0') != newpass.npos )
        throw std::invalid_argument("Neues Passwort enthält ungültige Zeichen!\nBitte ein anderes Passwort verwenden!");
    else if ( newpass == getpass() )
        return; // Gleiches Passwort wie vorher, nichts tun

    fromFile(); // Zur Sicherheit, um nicht aus Versehen neue Passwörter von anderen Admins zu überschreiben

    if ( newpass == std_pass )
        allpass.erase( nutzername ); // Wenn wieder auf Standard-Passwort gewechselt löschen
    else
        allpass[ nutzername ] = std::move( newpass );

    toFile();

    klog("Neues Passwort gesetzt!");
}
