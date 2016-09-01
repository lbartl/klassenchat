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

// Diese Datei wird regelmäßig aufgerufen und prüft, ob alles im Chat noch "normal" ist

#include "chat.hpp"
#include "warnung.hpp"
#include "simpledialog.hpp"
#include "chatverwaltung.hpp"
#include "filesystem.hpp"
#include "klog.hpp"

/// Prüft, ob spezielle Dinge gemacht werden sollen, und schreibt diese in #nextUiThing.
/**
 * Wird alle 0,1 Sekunden von pruefen_thread() aufgerufen.
 *
 * Schreibt Informationen zum Beenden des Chats, zu Warnungen, Informationen, Änderungen von Nutzer::admin und zur Erstellung eines neuen Privatchats in #nextUiThing.
 * Diese werden dann von pruefen_main() verarbeitet und die entsprechende Aktion ausgeführt.
 */
void Chat::pruefen_files() {
    using static_paths::warnfile;

    if ( static_paths::alltfile.exist() ) { // Admin hat überall den Chat geschlossen
        klog("terminate-all");
        nextUiThing.newTyp( UiThing::terminate );
    } else if ( terminatefile.exist() ) { // Admin hat meinen Benutzernamen entfernt
        unique_lock lock ( nextUiThing.mtx );
        nextUiThing.newTyp( lock, UiThing::entfernt );

        new ( nextUiThing.first() ) std::string( terminatefile.readLine() ); // wer mich entfernt hat

        terminatefile.remove();
    } else if ( checkfile.exist() ) { // Zeigen, dass ich noch im Chat bin, indem die Datei entfernt wird
        klog("Lösche checkfile...");
        checkfile.remove();
    } else if ( ! nutzer_ich.x_plum && warnfile.exist() ) { // Warnung
        std::ifstream is = warnfile.istream();

        while ( true ) {
            size_t currnummer;
            is >> currnummer;
            if ( currnummer == nutzer_ich.nummer ) // bereits empfangen
                break;
            else if ( ! is ) { // noch nicht empfangen
                is.close();
                warnfile.ostream( true ) << nutzer_ich.nummer << ' ';
                klog("Warnung empfangen!");
                nextUiThing.newTyp( UiThing::Warnung );
                break;
            }
        }
    }

    if ( infofile.exist() ) { // andere Info an mich
        switch ( infofile.readChar() ) {
        case '1': // Zum Admin werden
            klog("Werde zum Admin...");
            nutzer_verwaltung.flip_admin();
            nextUiThing.newTyp( UiThing::ToAdmin );
            break;
        case '0': // Zum normalen Nutzer werden
            klog("Werde zum normalen Nutzer...");
            nutzer_verwaltung.flip_admin();
            nextUiThing.newTyp( UiThing::FromAdmin );
            break;
        case 'i': { // Admin hat mir eine Info gesendet
            klog("Info von Admin empfangen!");

            std::string const inhalt = infofile.readAll();
            size_t const line_end = inhalt.find('\n');

            QString text = "Der Administrator " + QString::fromUtf8( inhalt.c_str() + 1, line_end - 1 ) + " schreibt:\n\n"
                          + QString::fromUtf8( inhalt.c_str() + line_end + 1, inhalt.length() - line_end - 1 );

            unique_lock lock ( nextUiThing.mtx );
            nextUiThing.newTyp( lock, UiThing::Dialog );

            new ( nextUiThing.first() ) QString("Information");
            new ( nextUiThing.second() ) QString( std::move( text ) );
        } break;
        default: // Neuer Chat, jemand lädt mich ein
            Datei chatdatei;
            size_t partner;
            infofile.istream() >> chatdatei >> partner;

            unique_lock lock ( nextUiThing.mtx );
            nextUiThing.newTyp( lock, UiThing::Privatchat );

            new ( nextUiThing.first() ) Datei( std::move( chatdatei ) );
            new ( nextUiThing.second() ) size_t { partner };
        }

        infofile.remove();
    }
}

/// Aktionen, die von Threads in #nextUiThing festgelegt wurden, ausführen.
/**
 * @returns true, wenn %Chat beendet oder neugestartet werden soll, sonst false.
 *
 * Wird alle 0,1 Sekunden von main_thread() aufgerufen.
 *
 * Überprüft #nextUiThing und führt dann die entsprechende Aktion aus.
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
                    chat_verwaltung.entfernt( entferner.c_str()+2 );
                else
                    chat_verwaltung.beenden();

                SimpleDialog dialog ( "ARSCHLOCH", "Der Chat ist nicht für dich, Arschloch!!!", this );
                dialog.setWindowModality( Qt::ApplicationModal );
                dialog.exec();
            } else {
                klog("entfernt");
                chat_verwaltung.entfernt( entferner.c_str()+1 );
            }

            return true;
        }
    case UiThing::Warnung: { // Warnung anzeigen
        Warnung*const warn = new Warnung( this );
        warn->setAttribute( Qt::WA_DeleteOnClose );
        warn->show();
    } break;
    case UiThing::ToAdmin: // Admin werden
        ui.menuAdmin->setEnabled( chat_verwaltung.imKlassenchat() ); // Nur im Klassenchat anzeigen
        ui.action_berall_den_Chat_beenden->setEnabled( true );
        break;
    case UiThing::FromAdmin: // normaler Nutzer werden
        ui.menuAdmin->setEnabled( false );
        ui.action_berall_den_Chat_beenden->setEnabled( false );
        break;
    case UiThing::Dialog: // Dialog anzeigen
        createDialog( *nextUiThing.first <QString>(), *nextUiThing.second <QString>(), this );
        break;
    case UiThing::Privatchat: // neuen Privatchat erstellen
        chat_verwaltung.newChat( std::move( *nextUiThing.first <Datei>() ), *nextUiThing.second <size_t>() );
    default:
        break;
    }

    nextUiThing.destruct();
    return false;
}
