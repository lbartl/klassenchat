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

// Diese Datei definiert das Singleton NutzerVerwaltung

#include "chat.hpp"
#include "chatverwaltung.hpp"
#include "pc_nutzername.hpp"
#include "global.hpp"
#include <QDebug>

NutzerVerwaltung& nutzer_verwaltung = NutzerVerwaltung::getInstance();

void NutzerVerwaltung::einlesen() {
    if ( ! file.exist() )
        file.ostream() << '0';

    std::ifstream is = file.istream();
    bool vorher_geloescht, admin;
    size_t nummer;

    for ( iterator it = begin();; ++it ) {
        is >> nummer;

        if ( is.get() == EOF ) { // Abbruchbedingung
            next_nummer = nummer;

            while ( it != end() ) {
                qDebug() << "Nutzer gelöscht: " << it->nummer << ' ' << it->admin << it->x_plum << it->nutzername.c_str() << ' ' << it->pc_nutzername.c_str();
                chat_verwaltung.nutzerGeloescht( *it );
                it = alle_nutzer.erase( it );
            }

            return;
        }

        admin = is.get() == '1'; // Ob der Nutzer ein Admin ist

        do {
            vorher_geloescht = false;

            if ( it == end() ) { // Neuer Nutzer
                bool x_plum = is.get() == '1';
                std::string nutzername, pc_nutzername;
                is >> nutzername;
                is.ignore( 1 ); // Leerzeichen ignorieren
                std::getline( is, pc_nutzername ); // Pc-Nutzername kann auch Leerzeichen enthalten

                qDebug() << "Neuer Nutzer: " << nummer << ' ' << admin << x_plum << nutzername.c_str() << ' ' << pc_nutzername.c_str();
                it = alle_nutzer.emplace_hint( it, admin, x_plum, std::move( nutzername ), std::move( pc_nutzername ), nummer ); // Hinzufügen
            } else if ( *it < nummer ) { // Nutzer gelöscht
                qDebug() << "Nutzer gelöscht: " << it->nummer << ' ' << it->admin << it->x_plum << it->nutzername.c_str() << ' ' << it->pc_nutzername.c_str();
                chat_verwaltung.nutzerGeloescht( *it ); // Falls der Nutzer einen Privatchat mit mir hatte, wird dieser gelöscht
                it = alle_nutzer.erase( it ); // Aktuelles Element löschen
                vorher_geloescht = true;
            } else {
                if ( admin != it->admin ) // Admin-Status des Nutzers hat sich geändert
                    it->admin_priv = admin;

                is.ignore( std::numeric_limits<std::streamsize>::max(), '\n' ); // Zum nächsten Nutzer gehen
            }
        } while ( vorher_geloescht );
    }
}

void NutzerVerwaltung::schreiben() {
    std::ofstream os = file.ostream();

    for ( Nutzer const& currnutzer : alle_nutzer )
        os << currnutzer.nummer << ' ' << currnutzer.admin << currnutzer.x_plum << currnutzer.nutzername << ' ' << currnutzer.pc_nutzername << '\n';

    os << next_nummer;
}

/**
 * @param x_plum Ob ich im Plum-Chat bin
 * @param nutzername Mein Benutzername
 */
void NutzerVerwaltung::makeNutzerIch( bool const x_plum, std::string nutzername ) {
    lock_guard lock ( mtx );
    file_mtx_lock f_lock ( file_mtx );

    einlesen();

    ich = alle_nutzer.emplace_hint( end(), ! x_plum && enthaelt( Chat::std_admins, nutzername ),
                                    x_plum, std::move( nutzername ), get_pc_nutzername(), next_nummer++ ); // Nutzer hinzufügen

    file.ostream( true ) << ' ' << ich->admin << x_plum << ich->nutzername << ' ' << ich->pc_nutzername << '\n' << next_nummer;
}

void NutzerVerwaltung::flip_x_plum() {
    lock_guard lock ( mtx );
    file_mtx_lock f_lock ( file_mtx );

    einlesen();

    iterator ich_alt = ich;
    ich = alle_nutzer.emplace_hint( end(), ich_alt->admin, !ich_alt->x_plum, ich_alt->nutzername, ich_alt->pc_nutzername, next_nummer++ );
    alle_nutzer.erase( ich_alt );

    schreiben();
}

void NutzerVerwaltung::flip_admin() {
    lock_guard lock ( mtx );
    file_mtx_lock f_lock ( file_mtx );

    einlesen();

    ich->admin_priv = !ich->admin; // admin flippen

    schreiben();
}

void NutzerVerwaltung::herausnehmen() {
    lock_guard lock ( mtx );
    file_mtx_lock f_lock ( file_mtx );

    einlesen();

    alle_nutzer.erase( ich );
    ich = end();

    schreiben();
}
