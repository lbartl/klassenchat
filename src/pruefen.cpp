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
#include "filesystem.hpp"
#include "klog.hpp"

using namespace static_paths;

/// Prüfen, ob mein Benutzername vergeben oder verboten ist.
/**
 * @returns true wenn vergeben oder verboten, sonst false.
 *
 * Wenn mein Benutzername vergeben oder verboten ist, wird ein Dialog angezeigt.
 */
bool Chat::vergeben() {
    klog("Überprüfen ob Nutzername vergeben oder verboten ist...");

    if ( nutzername == "" ) {
        qWarning("Keinen Namen eingegeben!");
        return true;
    }

    if ( ! verbotenfile.exist() )
        verbotenfile.write("Ich"); // "Ich" ist immer verboten

    if ( Hineinschreiben( toBenutzername_str( x_plum, nutzername ), nutzerfile ).hineingeschrieben() ) { // Prüfen ob Benutzername vergeben ist
        checkfile.touch();
        this_thread::sleep_for( 0.5s ); // Warten bis die Datei von dem Nutzer gelöscht wird

        if ( checkfile.exist() ) { // Bei diesem Nutzer ist der Chat abgestürzt
            checkfile.remove();
            Hineinschreiben( toBenutzername_str( x_plum, nutzername ), adminfile ).herausnehmen(); // Falls der Nutzer ein Admin war, aber kein std_admin
            lockfile_mtx.lock();
                lockfile -> remove();
            lockfile_mtx.unlock();
            return false;
        } else {
            klog("Nutzername vergeben!");
            createDialog( "Fehler", "Es ist bereits jemand mit diesem Benutzernamen angemeldet!\nBitte einen anderen Benutzernamen wählen!", this, true );
            return true;
        }
    } else if ( Hineinschreiben( verbotenfile ).hineingeschrieben( QString::fromStdString( nutzername ), true ) ) { // Prüfen ob Benutzername verboten ist
        klog("Nutername verboten!");
        createDialog( "Fehler", "Ein Admin hat diesen Benutzernamen verboten!\nBitte einen anderen Benutzernamen wählen!", this, true );
        return true;
    } else
        return false;
}

/// Aktionen, die von Threads in #nextUiThing festgelegt wurden, ausführen.
/**
 * @returns true, wenn %Chat beendet oder neugestartet werden soll, sonst false.
 *
 * Wird alle 0,1 Sekunden von main_thread() aufgerufen.
 *
 * Überprüft nextUiThing.typ und führt dann die entsprechende Aktion aus.
 */
bool Chat::pruefen_main() {
    bool const restart = flags[x_restart];

    if ( restart || flags[x_close] ) { // Ich beende selbst
        klog( restart ? "Neustart" : "Beenden" );
        Datei_lock_append( *chatfile_all, chatfile_all_mtx, nutzername + " hat den Chat verlassen" );
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
        return true;
    case UiThing::entfernt: // Ich wurde entfernt
        if ( flags[locked] ) { // Kann nicht entfernt werden, stattdessen diesen Benutzer entfernen
            klog("Jemand wollte mich entfernen, diesen Nutzer in den Entfernen-Dialog stellen...");
            entfernen( *nextUiThing.first <std::string>() );
            break;
        } else {
            klog("entfernt");
            Datei_lock_append( *chatfile_all, chatfile_all_mtx, nextUiThing.first <std::string>() -> append(" hat " + nutzername + " entfernt") );
            return true;
        }
    case UiThing::Warnung: { // Warnung anzeigen
        Warnung*const warn = new Warnung( this );
        warn -> setAttribute( Qt::WA_DeleteOnClose );
        warn -> show();
    } break;
    case UiThing::ToAdmin: // Admin werden
        flags.set( admin );
        ui.menuAdmin -> setEnabled( flags[chatall] ); // Nur im Klassenchat anzeigen
        ui.action_berall_den_Chat_beenden -> setEnabled( true );
        break;
    case UiThing::FromAdmin: // normaler Nutzer werden
        flags.reset( admin );
        ui.menuAdmin -> setEnabled( false );
        ui.action_berall_den_Chat_beenden -> setEnabled( false );
        break;
    case UiThing::Dialog: // Dialog anzeigen
        createDialog( *nextUiThing.first <QString>(), *nextUiThing.second <QString>(), this );
        break;
    case UiThing::Privatchat: // neue Chataction erstellen
        new_chat( std::move( *nextUiThing.first <Datei>() ), std::move( *nextUiThing.second <std::string>() ) );
    }

    nextUiThing.destruct();
    return false;
}
