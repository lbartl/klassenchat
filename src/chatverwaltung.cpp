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

#include "chatverwaltung.hpp"
#include "personalo.hpp"
#include "global.hpp"
#include "filesystem.hpp"
#include "klog.hpp"

ChatVerwaltung& chat_verwaltung = ChatVerwaltung::getInstance();

ChatVerwaltung::Chatfile ChatVerwaltung::chatfile_norm {"./verlauf.jpg"}, ///< %Datei, in der der normale %Chat gespeichert ist.
                         ChatVerwaltung::chatfile_plum {"./baum.jpg"}; ///< %Datei, in der der Plum-Chat gespeichert ist.

/**
 * @param partner Nutzername des Chatpartners
 *
 * Neuen %Privatchat mit diesem Chatpartner erstellen und öffnen.
 *
 * \callgraph
 */
void ChatVerwaltung::makeChat( std::string const& partner ) {
    shared_lock l1 ( nutzer_verwaltung.read_lock() );
    lock_guard l2 ( privatchats_mtx );

    Nutzer const*const chatpartner = nutzer_verwaltung.getNutzer( nutzer_ich.x_plum, partner );

    if ( ! chatpartner ) // Nutzername ist nicht im Chat
        return;

    auto const chata_it = std::find_if( privatchats.begin(), privatchats.end(), // Privatchat mit partner finden und Iterator darauf zurückgeben
                                        [chatpartner] ( Privatchat const& currchat ) { return &currchat.partner == chatpartner; } );

    if ( chata_it != privatchats.end() ) {
        KLOG << "Privatchat mit " << partner << " existiert bereits! Wechsle in diesen Chat..." << endl;
        openChat( &*chata_it );
        return;
    }

    Datei file = static_paths::senddir / std::to_string( chatpartner->nummer ) + '_' + std::to_string( nutzer_ich.nummer ) + ".jpg";
    file.ostream() << "Privatchat von " << nutzer_ich.nutzername << " und " << partner << '\n';
    makeToNutzerDatei( static_paths::infodir, *chatpartner ).ostream() << file << nutzer_ich.nummer; // Info an Partner schreiben was die Chatdatei ist und wer sein Chatpartner ist (also ich)

    neuerChat( std::move( file ), *chatpartner );
}

/**
 * @param chatdatei Chatdatei des Privatchats
 * @param partner_nummer Nummer des Chatpartners
 *
 * Neuen %Privatchat mit diesem Chatpartner öffnen.
 *
 * \callgraph
 */
void ChatVerwaltung::newChat( Datei chatdatei, size_t const partner_nummer ) {
    shared_lock l1 ( nutzer_verwaltung.read_lock() );
    lock_guard l2 ( privatchats_mtx );

    Nutzer const*const partner = nutzer_verwaltung.getNutzer( partner_nummer );

    if ( partner )
        neuerChat( std::move( chatdatei ), *partner );
}

/**
 * @param chatdatei Chatdatei des Privatchats
 * @param partner Chatpartner
 *
 * Erstellt ein neues Objekt von Privatchat, speichert es in #privatchats und öffnet dann diesen %Privatchat.
 */
void ChatVerwaltung::neuerChat( Datei chatdatei, Nutzer const& partner ) {
    privatchats.emplace_back( std::move( chatdatei ), partner ); // In Liste speichern
    Privatchat*const chat = &*privatchats.rbegin(); // Zeiger auf das letzte Element

    menuChats->insertAction( ui_sep, &chat->action ); // Aktion vor den Seperator stellen
    QObject::connect( &chat->action, &QAction::triggered, [this, chat] () { openChat( chat ); } );

    openChat( chat ); // Chat öffnen

    KLOG << "Neuen Privatchat mit " << partner.nutzername << " erstellt!" << endl;
}

/**
 * @param partner Nutzername des Chatpartners
 *
 * Öffnet den %Privatchat mit diesem Chatpartner.
 * Wenn dieser %Privatchat noch nicht existiert, wird ein neues Objekt von PersonalO mit diesem Chatpartner erstellt.
 */
void ChatVerwaltung::changeChat( std::string const& partner ) {
    lock_guard lock ( privatchats_mtx );
    auto const chata_it = std::find_if( privatchats.begin(), privatchats.end(), // Privatchat mit partner finden und Iterator darauf zurückgeben
                                        [&partner] ( Privatchat const& currchat ) { return currchat.partner.nutzername == partner; } );

    if ( chata_it != privatchats.end() ) {
        klog("Existiert schon, öffne Chat...");
        openChat( &*chata_it );
    } else {
        klog("Existiert noch nicht, neuer Chat...");

        PersonalO* p1 = new PersonalO( partner, dynamic_cast <QWidget*> ( menuChats->parent() ) );
        p1->setAttribute( Qt::WA_DeleteOnClose );
        p1->show();
    }
}

void ChatVerwaltung::schreibeNachricht( std::string const& nachricht ) {
    lock_guard lock ( chatfile_mtx );
    file_mtx_lock f_lock ( chatfile->file_mtx );

    std::string::const_iterator str_pos = nachricht.cbegin(),
                                line_end;
    std::string::const_iterator const str_end = nachricht.cend();
    bool first = true;

    std::ofstream chatdatei = chatfile->file.ostream( true );

    do { // Nachricht Zeile für Zeile in Datei schreiben
        if ( first )
            first = false;
        else
            str_pos = line_end + 1;

        line_end = std::find( str_pos, str_end, '\n' );

        static constexpr char const* newline_str {"\\n"};

        if ( ! enthaelt_nur( str_pos, line_end, ' ' ) && ! std::equal( str_pos, line_end, newline_str, newline_str + 2 ) )
        { // Wenn kein Zeichen, nur Leerzeichen oder das Newline Zeichen eingegeben, nur neue Zeile ausgeben
            chatdatei << nutzer_ich.nutzername << ": ";
            chatdatei.write( &*str_pos, line_end - str_pos );
        }

        chatdatei.put('\n');
    } while ( line_end != str_end );
}

void ChatVerwaltung::flip_x_plum() {
    lock_guard l1 ( privatchats_mtx ),
               l2 ( chatfile_mtx );

    std::string normtext, plumtext;

    if ( nutzer_ich.x_plum ) {
        chatfile_all = &chatfile_plum;
        plumtext = nutzer_ich.nutzername + " hat den Chat betreten";
        normtext = nutzer_ich.nutzername + " hat in den Plum-Chat gewechselt";
   } else { // Kommt vom Plum-Chat
        chatfile_all = &chatfile_norm;
        plumtext = nutzer_ich.nutzername + " hat den Chat verlassen";
        normtext = nutzer_ich.nutzername + " hat in diesen Chat gewechselt";
    }

    chatfile = chatfile_all;
    privatchats.clear(); // Alle Privatchats löschen (Chatdateien werden von Chatpartnern entfernt)

    file_mtx_lock l3 ( chatfile_norm.file_mtx ),
                  l4 ( chatfile_plum.file_mtx );

    chatfile_norm.file.append( normtext );
    chatfile_plum.file.append( plumtext );
}

void ChatVerwaltung::nutzerGeloescht( Nutzer const& nutzer ) {
    lock_guard l1 ( privatchats_mtx ),
               l2 ( chatfile_mtx );

    auto const chata_it = std::find_if( privatchats.begin(), privatchats.end(), // Privatchat mit nutzer finden und Iterator darauf zurückgeben
                                        [&nutzer] ( Privatchat const& currchat ) { return &currchat.partner == &nutzer; } );

    if ( chata_it == privatchats.end() ) // gelöschter Nutzer hatte keinen Privatchat mit mit
        return;

    if ( &chata_it->chatfile == chatfile ) { // Ich befinde mich gerade im Chat mit dem gelöschten Nutzer, in den Klassenchat wechseln
        menuAdmin->setEnabled( nutzer_ich.admin );
        chatfile = chatfile_all;
    }

    chata_it->chatfile.remove(); // Chatdatei löschen
    privatchats.erase( chata_it ); // Aus Liste löschen

    text.first = true;
    text.second = QString::fromStdString( "Der Nutzer " + nutzer.nutzername + " hat den Chat verlassen!\n"
                                          "Der Privatchat mit " + nutzer.nutzername + " wurde gelöscht!");
}
