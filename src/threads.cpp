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

// Diese Datei steuert das Ausführen der Threads

#include "chat.hpp"
#include "chatverwaltung.hpp"
#include "filesystem.hpp"
#include "klog.hpp"

namespace {
    std::atomic <uint_fast8_t> threads_stop;
}

/// Startet nutzer_thread() und pruefen_thread() jeweils in einem eigenen Thread.
void Chat::start_threads() {
    threads_stop = 0;

    std::thread( [this] () { aktualisieren_thread(); } ).detach();
    std::thread( [this] () { nutzer_thread(); } ).detach();
    std::thread( [this] () { pruefen_thread(); } ).detach();

    klog("Threads gestartet!");
}

/// Beendet aktualisieren_thread(), nutzer_thread() und pruefen_thread()
void Chat::stop_threads() {
    ++threads_stop;

    while ( threads_stop != 4 ) // warten bis sich alle Threads beendet haben
        this_thread::sleep_for( 100ms );

    klog("Threads beendet!");
}

#define UNTIL_STOP while ( this_thread::sleep_for( 100ms ), threads_stop == 0 ) // Solange Thread nicht beendet werden soll, 0,1 Sekunden warten und dann Schleifenkörper ausführen

/// Aktualisiert den Chatverlauf.
/**
 * Alle 0,1 Sekunden wird #chatfile eingelesen und mit #inhalt verglichen.
 * Wenn #inhalt identisch mit dem ersten Teil von #chatfile is, wird inhalt.length() in #nextUiThing geschrieben.
 * Andernfalls wird 0 in #nextUiThing geschrieben.
 *
 * Alle 100 Sekunden wird 1 in #nextUiThing geschrieben.
 */
void Chat::aktualisieren_thread() {
    uint_fast16_t i = 0; // Alle 100 Sekunden (jedes 1000. Mal) wird der gesamte Chatverlauf aktualisiert (x_neu), da manchmal Fehler auftreten

    UNTIL_STOP
        if ( ++i == 1000 ) {
            unique_lock lock ( nextUiThing.mtx );
            nextUiThing.newTyp( lock, UiThing::aktualisieren );

            new ( nextUiThing.first() ) size_t { 1 };

            i = 0;
        } else {
            std::string inhalt_new = chat_verwaltung.einlesen(); // Datei einlesen

            if ( inhalt_new != inhalt ) { // Chatverlauf hat sich verändert
                unique_lock lock ( nextUiThing.mtx );
                nextUiThing.newTyp( lock, UiThing::aktualisieren );

                new ( nextUiThing.first() ) size_t { std::equal( inhalt.begin(), inhalt.end(), inhalt_new.begin() ) ? inhalt.length() : 0 };

                inhalt.swap( inhalt_new );
            }
        }

    ++threads_stop;
}

/// Aktualisiert die Nutzerverwaltung.
/**
 * Alle 0,1 Sekunden wird überprüft, ob #lockfile existiert, wenn nicht, wird es erstellt.
 *
 * Alle 0,5 Sekunden wird #nutzer_verwaltung aktualisiert und check_all_chats() aufgerufen.
 *
 * Beim Beenden wird #lockfile gelöscht und NutzerVerwaltung::herausnehmen() aufgerufen.
 */
void Chat::nutzer_thread() {
    uint_fast8_t i = 0; // nutzer_verwaltung wird nur jede halbe Sekunde (jedes 5. Mal) aktualisiert

    UNTIL_STOP {
        lock_guard lock ( nutzer_mtx );

        if ( ! lockfile->exist() ) {
            file_mtx_lock f_lock ( lockfile_mtx );
            lockfile->touch();
        }

        if ( ++i == 5 ) {
            i = 0;
            nutzer_verwaltung.aktualisieren();
            QString*const text = chat_verwaltung.getText();

            if ( text ) { // Ein Privatchat wurde gelöscht
                unique_lock lock ( nextUiThing.mtx );
                nextUiThing.newTyp( lock, UiThing::Dialog );

                new ( nextUiThing.first() ) QString("Privatchat gelöscht");
                new ( nextUiThing.second() ) QString( std::move( *text ) );
            }
        }
    }

    lockfile_mtx.lock();
        lockfile->remove();
    lockfile_mtx.unlock();

    nutzer_verwaltung.herausnehmen();

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
 * Schreibt Informationen zum Beenden des Chats, zu Warnungen, Informationen, Änderungen von Nutzer::admin und zur Erstellung eines neuen Privatchats in #nextUiThing.
 * Diese werden dann von pruefen_main() verarbeitet und die entsprechende Aktion ausgeführt.
 */
void Chat::pruefen_thread() {
    using namespace static_paths;
    using std::string;

#ifndef WIN32
    struct sigaction sig_ac;
    sig_ac.sa_handler = [] ( int ) { signal_stop = true; };
    sigemptyset( &sig_ac.sa_mask );
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

                string const inhalt = infofile.readAll();
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

    ++threads_stop;
}
