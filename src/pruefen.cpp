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

// Diese Datei wird regelmäßig von aktualisieren.cpp aufgerufen und prüft, ob alles im Chat noch "normal" ist

#include "chat.hpp"
#include "warnung.hpp"
#include "simpledialog.hpp"
#include "chatverwaltung.hpp"
#include "filesystem.hpp"
#include "klog.hpp"

using namespace static_paths;

/// Aktionen, die von Threads in #nextUiThing festgelegt wurden, ausführen.
/**
 * @returns true, wenn %Chat beendet oder neugestartet werden soll, sonst false.
 *
 * Wird alle 0,1 Sekunden von main_thread() aufgerufen.
 *
 * Überprüft nextUiThing.typ und führt dann die entsprechende Aktion aus.
 */
bool Chat::pruefen_main() {
    if ( flags[x_restart] || flags[x_close] ) { // Ich beende selbst
        klog( flags[x_restart] ? "Neustart" : "Beenden" );
        chat_verwaltung.beenden();
        return true;
    }

    // nextUiThing überprüfen (weitere Erklärungen bei der Definition von UiThing, chat.hpp)
    lock_guard lock ( nextUiThing.mtx );

    switch ( nextUiThing.getTyp() ) {
    case UiThing::nichts: // Nichts tun
        if ( flags[x_reload] ) { // Chatverlauf manuell aktualisieren
            verlauf_up( 0 );
            flags.reset( x_reload );
        }
        return false;
    case UiThing::aktualisieren: // Chatverlauf aktualisieren
        verlauf_up( *nextUiThing.first <size_t>() );
        break;
    case UiThing::terminate: // Chat beenden
        chat_verwaltung.beenden();
        return true;
    case UiThing::entfernt: // Ich wurde entfernt
        if ( flags[locked] ) { // Kann nicht entfernt werden, stattdessen diesen Benutzer entfernen
            klog("Jemand wollte mich entfernen, diesen Nutzer in den Entfernen-Dialog stellen...");
            entfernen( nextUiThing.first <std::string>()->erase( 0, 1 ) );
            break;
        } else {
            std::string const& entferner = *nextUiThing.first <std::string>();

            if ( entferner[0] == 'x' ) { // Pc-Nutzername gesperrt statt einfach nur entfernt
                klog("Admin hat meinen Pc-Nutzernamen gesperrt!");

                if ( ( entferner[1] == '1' ) == nutzer_ich.x_plum ) // Wenn Admin im gleichen Chat wie ich ist
                    chat_verwaltung.entfernt( nextUiThing.first <std::string>()->c_str()+2 );

                SimpleDialog dialog ( "ARSCHLOCH", "Der Chat ist nicht für dich, Arschloch!!!", this );
                dialog.setWindowModality( Qt::ApplicationModal );
                dialog.exec();
            } else {
                klog("entfernt");
                chat_verwaltung.entfernt( nextUiThing.first <std::string>()->c_str()+1 );
            }

            return true;
        }
    case UiThing::Warnung: { // Warnung anzeigen
        Warnung*const warn = new Warnung( this );
        warn -> setAttribute( Qt::WA_DeleteOnClose );
        warn -> show();
    } break;
    case UiThing::ToAdmin: // Admin werden
        ui.menuAdmin -> setEnabled( chat_verwaltung.imKlassenchat() ); // Nur im Klassenchat anzeigen
        ui.action_berall_den_Chat_beenden -> setEnabled( true );
        break;
    case UiThing::FromAdmin: // normaler Nutzer werden
        ui.menuAdmin -> setEnabled( false );
        ui.action_berall_den_Chat_beenden -> setEnabled( false );
        break;
    case UiThing::Dialog: // Dialog anzeigen
        createDialog( *nextUiThing.first <QString>(), *nextUiThing.second <QString>(), this );
        break;
    case UiThing::Privatchat: // neuen Privatchat erstellen
        chat_verwaltung.newChat( std::move( *nextUiThing.first <Datei>() ), *nextUiThing.second <size_t>() );
    }

    nextUiThing.destruct();
    return false;
}
