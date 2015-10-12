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

// Diese Datei steuert das Ausführen der Threads

#include "chat.hpp"
#include "filesystem.hpp"
#include "klog.hpp"

namespace {
    std::atomic <uint_fast8_t> threads_stop;
}

/// Startet nutzer_thread() und pruefen_thread() jeweils in einem eigenen Thread.
void Chat::start_threads() {
    threads_stop = 0;

    std::thread( [this] () { nutzer_thread(); } ).detach();
    std::thread( [this] () { pruefen_thread(); } ).detach();

    klog("Threads gestartet!");
}

/// Beendet nutzer_thread() und pruefen_thread().
void Chat::stop_threads() {
    ++threads_stop;

    while( threads_stop != 3 ) // warten bis sich alle Threads beendet haben
        this_thread::sleep_for( 0.1s );

    klog("Threads beendet!");
}

#define UNTIL_STOP while( this_thread::sleep_for( 0.1s ), threads_stop == 0 ) // Solange Thread nicht beendet werden soll, 0,1 Sekunden warten und dann Schleifenkörper ausführen

/// Aktualisiert die Nutzerverwaltung.
/**
 * Alle 0,1 Sekunden wird überprüft, ob #lockfile existiert, wenn nicht, wird es erstellt.
 *
 * Alle 0,5 Sekunden werden #nutzer_h und #admins_h aktualisiert und check_all_chats() aufgerufen.
 *
 * Beim Starten wird automatisch mein Name in #nutzer_h und #admins_h (wenn ich #admin bin) hineingeschrieben.
 *
 * Beim Beenden wird #lockfile gelöscht und mein Name aus #nutzer_h und #admins_h herausgenommen.
 */
void Chat::nutzer_thread() {
    nutzer_h -> reinschreiben();

    if ( admins_h ) admins_h -> reinschreiben();

    uint_fast8_t i = 0; // nutzer_h und admins_h werden nur jede halbe Sekunde (jedes 5. Mal) aktualisiert

    UNTIL_STOP {
        lock_guard lock ( nutzer_mtx );

        if ( ! lockfile -> exist() ) {
            file_mtx_lock f_lock ( lockfile_mtx );
            lockfile -> touch();
        }

        if ( ++i == 5 ) {
            i = 0;

            nutzer_h_mtx.lock();
                nutzer_h -> aktualisieren();
            nutzer_h_mtx.unlock();

            if ( admins_h ) { // Wenn admins_h kein nullptr ist, also wenn ich Admin bin
                lock_guard lock ( admins_h_mtx );
                admins_h -> aktualisieren();
            }

            check_all_chats(); // privatchats.cpp
        }
    }

    lockfile_mtx.lock();
        lockfile -> remove();
    lockfile_mtx.unlock();

    nutzer_h -> herausnehmen();
    if ( admins_h ) admins_h -> herausnehmen();

    ++threads_stop;
}

#ifndef WIN32
# include <csignal>
namespace {
    bool volatile signal_stop = false; // zeigt an, ob SIGINT oder SIGTERM gefangen wurden
}
#endif

/// Prüft, ob spezielle Dinge gemacht werden sollen, und schreibt diese in #nextUiThing.
/**
 * Schreibt Informationen zum Beenden des %Chats, zu Warnungen, Informationen, Änderungen von #admin und zur Erstellung eines neuen Privatchats in #nextUiThing.
 * Diese werden dann von pruefen_main() verarbeitet und die entsprechende Aktion ausgeführt.
 */
void Chat::pruefen_thread() {
    using namespace static_paths;
    using std::string;

#ifndef WIN32
    struct sigaction sig_ac;
    sig_ac.sa_handler = [] ( int ) { signal_stop = true; };
    sig_ac.sa_flags = 0;

    if ( sigaction( SIGINT, &sig_ac, nullptr ) ||
         sigaction( SIGTERM, &sig_ac, nullptr ) )
        throw std::runtime_error("Fehler beim Aufruf von sigaction()!");
#endif

    UNTIL_STOP {
        lock_guard lock ( pruefen_mtx );

#ifndef WIN32
        if ( signal_stop ) { // SIGINT oder SIGTERM
            klog("Beenden nach Signal!");
            nextUiThing.newTyp( UiThing::terminate );
            break; // Thread beenden
        }
#endif
        if ( alltfile.exist() ) { // Admin hat überall den Chat geschlossen
            klog("terminate-all");
            nextUiThing.newTyp( UiThing::terminate );
            break; // Thread beenden
        } else if ( terminatefile.exist() ) { // Admin hat meinen Benutzernamen entfernt
            unique_lock lock ( nextUiThing.mtx );
            nextUiThing.newTyp( lock, UiThing::entfernt );

            new ( nextUiThing.first() ) string( terminatefile.readLine() ); // wer mich entfernt hat

            terminatefile.remove();
        } else if ( ! x_plum && warnfile.exist() ) { // Warnung
            Hineinschreiben warn1 ( nutzername_str, warnfile );

            if ( ! warn1.hineingeschrieben() ) {
                warn1.reinschreiben();
                klog("Warnung empfangen!");
                nextUiThing.newTyp( UiThing::Warnung );
            }
        }

        if ( infofile.exist() ) { // andere Info an mich
            switch ( infofile.readChar() ) {
            case '1': // Zum Admin werden
                klog("Werde zum Admin...");

                nutzer_mtx.lock();
                    admins_h = std::make_unique <Hineinschreiben> ( nutzername_str, adminfile );
                    admins_h -> reinschreiben();
                nutzer_mtx.unlock();

                nextUiThing.newTyp( UiThing::ToAdmin );
                break;
            case '0': // Zum normalen Nutzer werden
                klog("Werde zum normalen Nutzer...");

                nutzer_mtx.lock();
                    admins_h -> herausnehmen();
                    admins_h.reset();
                nutzer_mtx.unlock();

                nextUiThing.newTyp( UiThing::FromAdmin );
                break;
            case 'i': { // Admin hat mir eine Info gesendet
                klog("Info von Admin empfangen!");

                string inhalt = infofile.readAll();
                size_t line_end = inhalt.find('\n');

                if ( x_plum ) {
                    size_t klammer_pos = inhalt.find( '(', line_end );

                    if ( klammer_pos != string::npos ) {
                        inhalt.erase( klammer_pos, line_end - klammer_pos );
                        line_end = klammer_pos - 1;
                    }
                }

                QString text = "Der Administrator " + QString::fromUtf8( inhalt.c_str() + 1, line_end - 1 ) + " schreibt:\n\n"
                              + QString::fromUtf8( inhalt.c_str() + line_end + 1, inhalt.length() - line_end - 1 );

                unique_lock lock ( nextUiThing.mtx );
                nextUiThing.newTyp( lock, UiThing::Dialog );

                new ( nextUiThing.first() ) QString("Information");
                new ( nextUiThing.second() ) QString( std::move( text ) );
            } break;
            default: // Neuer Chat, jemand lädt mich ein
                Datei chatdatei;
                string partner;
                infofile.istream() >> chatdatei >> partner;

                unique_lock lock ( nextUiThing.mtx );
                nextUiThing.newTyp( lock, UiThing::Privatchat );

                new ( nextUiThing.first() ) Datei( std::move( chatdatei ) );
                new ( nextUiThing.second() ) string( std::move( partner ) );
            }

            infofile.remove();
        }
    }

    ++threads_stop;
}
