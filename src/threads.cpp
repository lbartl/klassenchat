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
 * Alle 0,1 Sekunden wird ChatVerwaltung::einlesen() aufgerufen und mit #inhalt verglichen.
 * Wenn #inhalt identisch mit dem ersten Teil ist, wird inhalt.length() in #nextUiThing (UiThing::aktualisieren) geschrieben.
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
 * Alle 0,5 Sekunden wird NutzerVerwaltung::aktualisieren() aufgerufen und überprüft ob ein Privatchat gelöscht wurde.
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

/// Prüft, ob spezielle Dinge gemacht werden sollen.
/**
 * Prüft, ob der %Chat nach einem Signal beendet werden soll oder ob ein Admin überall den %Chat geschlossen hat.
 * Wenn beides nicht zutrifft, wird pruefen_files() aufgerufen.
 */
void Chat::pruefen_thread() {
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
        if ( static_paths::alltfile.exist() ) { // Admin hat überall den Chat geschlossen
            klog("terminate-all");
            nextUiThing.newTyp( UiThing::terminate );
            break;
        }

        pruefen_files();
    }

    ++threads_stop;
}
