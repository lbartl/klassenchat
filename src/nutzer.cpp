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
#include "klog.hpp"

#ifdef WIN32
# include <lmcons.h>
# include <codecvt>
inline std::string get_pc_nutzername() {
    wchar_t name [UNLEN+1];
    DWORD size = UNLEN+1;
    return GetUserNameW( name, &size ) ? std::wstring_convert <std::codecvt_utf8 <wchar_t>>().to_bytes( name ) : "";
}
#else
# include <pwd.h>
inline std::string get_pc_nutzername() {
    passwd* pw = getpwuid( geteuid() );
    return pw ? pw->pw_name : "";
}
#endif

///\cond

NutzerVerwaltung& nutzer_verwaltung = NutzerVerwaltung::getInstance();

void NutzerVerwaltung::einlesen() {
    if ( ! file.exist() )
        file.ostream() << '0';

    std::ifstream is = file.istream();
    bool vorher_geloescht;
    size_t nummer;

    for ( iterator it = begin();; ++it ) {
        is >> nummer;

        if ( is.get() == EOF ) { // Abbruchbedingung
            next_nummer = nummer;
            alle_nutzer.erase( it, end() );
            return;
        }

        do {
            vorher_geloescht = false;

            if ( it == end() ) {
                bool x_plum = is.get() == '1',
                     admin = is.get() == '1';
                std::string nutzername, pc_nutzername;
                is >> nutzername >> pc_nutzername;

                KLOG << "Neuer Nutzer: " << nummer << ' ' << x_plum << admin << nutzername << ' ' << pc_nutzername << endl;
                it = alle_nutzer.emplace_hint( it, x_plum, admin, std::move( nutzername ), std::move( pc_nutzername ), nummer ); // Hinzufügen
            } else if ( *it < nummer ) {
                KLOG << "Nutzer gelöscht: " << it->nummer << ' ' << it->x_plum << it->admin << it->nutzername << ' ' << it->pc_nutzername << endl;
                it = alle_nutzer.erase( it ); // Aktuelles Element löschen
                vorher_geloescht = true;
            } else // Nutzer ist aktuell
                is.ignore( std::numeric_limits<std::streamsize>::max(), '\n' ); // Zum nächsten Nutzer gehen
        } while ( vorher_geloescht );
    }
}

void NutzerVerwaltung::schreiben() {
    std::ofstream os = file.ostream();

    for ( Nutzer const& currnutzer : alle_nutzer )
        os << currnutzer.nummer << ' ' << currnutzer.x_plum << currnutzer.admin << currnutzer.nutzername << ' ' << currnutzer.pc_nutzername << '\n';

    os << next_nummer;
}

void NutzerVerwaltung::makeNutzerIch( bool const x_plum, std::string nutzername ) {
    lock_guard lock ( mtx );
    file_mtx_lock f_lock ( file_mtx );

    einlesen();

    ich = alle_nutzer.emplace_hint( end(), x_plum, ! x_plum && enthaelt( Chat::std_admins, nutzername ),
                                    std::move( nutzername ), get_pc_nutzername(), next_nummer++ ); // Nutzer hinzufügen

    file.ostream( true ) << ' ' << x_plum << ich->admin << ich->nutzername << ' ' << ich->pc_nutzername << '\n' << next_nummer;
}

void NutzerVerwaltung::flip_x_plum() {
    lock_guard lock ( mtx );
    file_mtx_lock f_lock ( file_mtx );

    einlesen();

    alle_nutzer.erase( ich );
    ich = alle_nutzer.emplace_hint( end(), !ich->x_plum, ich->admin, ich->nutzername, ich->pc_nutzername, next_nummer++ );

    schreiben();
}

void NutzerVerwaltung::flip_admin() {
    lock_guard lock ( mtx );
    file_mtx_lock f_lock ( file_mtx );

    einlesen();

    alle_nutzer.erase( ich );
    ich = alle_nutzer.emplace_hint( end(), ich->x_plum, !ich->admin, ich->nutzername, ich->pc_nutzername, next_nummer++ );

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

///\endcond
