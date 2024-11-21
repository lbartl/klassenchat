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

// Diese Datei steuert das Öffnen der QDialogs

#include "chat.hpp"
#include "entfernen.hpp"
#include "simpledialog.hpp"
#include "nutzer_anz.hpp"
#include "personalo.hpp"
#include "infoopen.hpp"
#include "passwort.hpp"
#include "chatverwaltung.hpp"
#include <QDebug>

using std::string;
using std::string_view;

/// Zeigt eine Hilfe zu Tastenkombinationen und Kommandos an.
void Chat::hilfe_anz() {
    qDebug("Hilfe anzeigen...");

    QString kombi_str = "Tastenkombinationen:\n"
                        "F1: Diese Hilfe aufrufen\n"
                        "Strg+Q: Beenden\n"
                        "Strg+K: In den Klassenchat wechseln\n";

    QString kommand_str = "Befehle:\n"
                          "/help: Diese Hilfe aufrufen\n"
                          "/vordergrund: Immer im Vordergrund an- bzw. ausschalten\n"
                          "/exit: Beenden\n"
                          "/restart: Neustart\n"
                          "/reload: Chatverlauf neu laden\n"
                          "/chat <name>: In den Privatchat mit <name> wechseln\n";

    if ( nutzer_ich.admin ) {
        kombi_str += "Strg+T: Überall den Chat beenden\n";

        kommand_str += "/all: Überall den Chat beenden\n";

        if ( chat_verwaltung.imKlassenchat() ) {
            kommand_str += "/terminate <name>: <name> entfernen\n"
                           "/info <name>: An <name> eine Information senden\n"
                           "/schreibeinfo <text>: <text> als Information (grüne Zeile) schreiben\n"
                           "/warnung: Warnung senden\n"
                           "/reset: Chatverlauf löschen\n"
                           "/drin: Anzeigen, wer alles im Chat ist\n"
                           "/verboten: Verbotene Benutzernamen anzeigen und ändern\n"
                           "/verbotenpc: Verbotene Pc-Nutzernamen anzeigen und ändern\n"
                           "/plum: " + ui.actionIn_den_Plum_Chat_wechseln -> text().remove( 0, 1 ) + '\n';

            if ( flags[std_admin] )
                kommand_str += "/passwort: Mein Admin-Passwort ändern\n";
        }
    }

    createDialog( "Hilfe", kombi_str + '\n' + kommand_str, this );
}

/// Erstellt für den #oberadmin ein Objekt der Klasse Admin_anz, für andere Admins ein Objekt der Klasse Nutzer_anz.
void Chat::nutzer_anz() {
    qDebug("Nutzer anzeigen...");

    QDialog* anz_nutz = flags[x_oberadmin] ? static_cast <QDialog*> ( new Admin_anz( this ) ) // für Oberadmin
                                           : static_cast <QDialog*> ( new Nutzer_anz( this ) ); // für alle anderen Admins

    anz_nutz -> setAttribute( Qt::WA_DeleteOnClose );
    anz_nutz -> show();
}

/// Erstellt ein Objekt der Klasse Entfernen (Admin).
void Chat::entfernen( string_view const& name ) {
    Entfernen* entf1 = new Entfernen( name, this );
    entf1 -> setAttribute( Qt::WA_DeleteOnClose );
    entf1 -> show();
}

/// Erstellt ein Objekt der Klasse PersonalO mit einem bestimmten Chatpartner.
void Chat::personal_op( string const& partner ) {
    PersonalO* p1 = new PersonalO( partner, this );
    p1 -> setAttribute( Qt::WA_DeleteOnClose );
    p1 -> show();
}

/// Erstellt ein Objekt der Klasse InfoOpen mit einem bestimmten Empfänger (Admin).
void Chat::info_open( string const& an ) {
    InfoOpen* infoo1 = new InfoOpen( an, this );
    infoo1 -> setAttribute( Qt::WA_DeleteOnClose );
    infoo1 -> show();
}

/// Erstellt ein Objekt der Klasse Passwort, und gibt das eingegebene %Passwort an set_pass() weiter (#std_admin).
void Chat::openAdminPass() {
    Passwort* pass = new Passwort( this );
    pass -> setText("Neues Admin-Passwort:");
    pass -> setWindowTitle("Neues Admin-Passwort");

    connect( pass, &Passwort::eingegeben, this, &Chat::set_pass );

    pass -> setAttribute( Qt::WA_DeleteOnClose );
    pass -> show();
}
