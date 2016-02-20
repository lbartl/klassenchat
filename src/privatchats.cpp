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

// Diese Datei steuert Funktionen für Privatchats

#include "chat.hpp"
#include "simpledialog.hpp"
#include "filesystem.hpp"
#include "klog.hpp"

using std::string;

/// Prüfen, ob ein Privatchat mit einem bestimmten Chatpartner existiert.
/**
 * @param partner der Chatpartner
 * @returns true, wenn existiert, sonst false.
 *
 * Sucht in #chats_ac nach einer Chataction mit diesem Chatpartner.
 * Wenn eine gefunden wird, wird dieser Privatchat geöffnet.
 */
bool Chat::exist_chat( string const& partner ) {
    lock_guard lock ( chats_ac_mtx );
    auto chata_it = std::find_if( chats_ac.begin(), chats_ac.end(), // Chataction mit partner finden und Iterator darauf zurückgeben
                                  [&partner] ( Chataction const& currac ) { return currac.partner == partner; } );

    bool const exist = chata_it != chats_ac.end();
    if ( exist ) open_chat( &chata_it->datei ); // Wenn existiert, dann öffnen
    return exist;
}

/**
 * @param partner Benutzername des Chatpartners.
 *
 * Neuen Privatchat mit diesem Chatpartner erstellen und öffnen.
 *
 * \callgraph
 */
void Chat::make_chat( string partner ) {
    if ( exist_chat( partner ) ) { // Chat mit diesem partner existiert schon
        ios_base::sync_with_stdio( false );
        KLOG << "Privatchat mit " << partner << " existiert schon! In diesen Chat gewechselt!" << endl;
        return;
    }

    Datei file = static_paths::senddir / partner + '_' + nutzername;
    file += x_plum ? "_1.jpg" : "_0.jpg";

    file.ostream() << "Privatchat von " << nutzername << " und " << partner << '\n';

    makeToNutzerDatei( static_paths::infodir, x_plum, partner ).ostream() << file << nutzername; // Info an Partner schreiben was die Chatdatei ist und wer sein Chatpartner ist (also ich)

    new_chat( std::move( file ), std::move( partner ) );
}

/// Eine neue Chataction erstellen. Aufgerufen von make_chat() und pruefen2()
/**
 * @param dateichat Chatdatei des Privatchats
 * @param partner Chatpartner des Privatchats
 *
 * Erstellt eine neue Chataction in #chats_ac und öffnet dann diesen Privatchat.
 */
void Chat::new_chat( Datei dateichat, string partner ) {
    lock_guard lock ( chats_ac_mtx );
    Chataction& chatac { *chats_ac.emplace_after( chats_ac.before_begin(), std::move( dateichat ), std::move( partner ) ) }; // Aktion erstellen und speichern
    Datei const*const chatdatei = &chatac.datei; // Zeiger zu Chatdatei

    ui.menuChats -> insertAction( ui_sep, &chatac.action ); // Aktion vor den Seperator stellen
    connect( &chatac.action, &QAction::triggered, [this, chatdatei] () { open_chat( chatdatei ); } );

    open_chat( chatdatei ); // Chat öffnen

    ios_base::sync_with_stdio( false );
    KLOG << "Neuen Privatchat mit " << chatac.partner << " erstellt!" << endl;
}

/// Öffnet einen Privatchat.
void Chat::open_chat( Datei const*const chatdatei ) {
    ui.menuAdmin -> setEnabled( false );
    flags.reset( chatall );
    chatfile = chatdatei;
}

/// Öffnet den Klassenchat.
void Chat::klassenchat() {
    ui.menuAdmin -> setEnabled( flags[admin] ); // Wenn man ein Admin ist einschalten, sonst ausschalten
    flags.set( chatall ); // zeigt Funktionen wie senden_pruef(), dass man im Klassenchat ist
    chatfile = chatfile_all;
}

/// Privatchat öffnen oder erstellen.
/**
 * @param partner Chatpartner
 *
 * Öffnet den Privatchat mit diesem Chatpartner, wenn er nicht existiert, wird personal_op() mit diesem Chatpartner aufgerufen.
 */
void Chat::ch_chat( string const& partner ) {
    if ( exist_chat( partner ) )
        klog("Existiert schon, öffne Chat...");
    else {
        klog("Existiert noch nicht, neuer Chat...");
        personal_op( partner );
    }
}

/// Überprüfen, ob alle Privatchats noch existieren. Wird alle 0,5 Sekunden von nutzer_thread() aufgerufen
/**
 * Prüft für jeden Privatchat aus #chats_ac, ob er noch gültig ist,
 * indem geschaut wird, ob der Chatpartner in #nutzer_h steht.
 *
 * Wenn der Privatchat nicht mehr gültig ist, wird ein Dialog angezeigt und die entsprechende Chataction gelöscht.
 */
void Chat::check_all_chats() {
    lock_guard lock ( chats_ac_mtx );
    auto it_before = chats_ac.before_begin();

    for ( Chataction const& currac : chats_ac ) {
        lock_guard lock ( nutzer_h_mtx );

        if ( ! nutzer_h -> hineingeschrieben( x_plum, currac.partner ) ) { // nicht mehr gültig
            QString text = QString::fromStdString( "Der Nutzer " + currac.partner + " hat den Chat verlassen!\n"
                                                   "Der Privatchat mit " + currac.partner + " wurde gelöscht!" );
            if ( chatfile == &currac.datei )
                klassenchat();

            currac.datei.remove(); // Chatdatei löschen
            chats_ac.erase_after( it_before ); // Aus forward_list löschen

            unique_lock lock ( nextUiThing.mtx );
            nextUiThing.newTyp( lock, UiThing::Dialog );

            new ( nextUiThing.first() ) QString("Privatchat gelöscht");
            new ( nextUiThing.second() ) QString( std::move( text ) );

            break;
        } else
            ++it_before;
    }
}
