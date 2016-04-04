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

// Diese Datei steuert Kommandos und Nachrichten, die eingegeben wurden

#include "chat.hpp"
#include "simpledialog.hpp"
#include "filesystem.hpp"
#include "klog.hpp"
#include <boost/tokenizer.hpp>

using std::string;
using sep = boost::char_separator <char>;

/// Sendet eine Nachricht oder führt ein eingegebenes Kommando aus.
/**
 * Wird nach Eingabe einer Nachricht aufgerufen und wertet das Eingegebene aus.
 * Wenn die Eingabe mit einem '/' beginnt, wird sie als Kommando interpretiert und die entsprechende Aktion wird ausgeführt.
 * Wenn nicht, wird die Eingabe als Nachricht interpretiert und sie wird in #chatfile geschrieben.
 */
void Chat::senden_pruef() {
    ui.NachrichtB -> setFocus(); // Fokus zurück auf Input
    string const nachricht = ui.NachrichtB -> text().toStdString(); // Eingegebener Text

    if ( nachricht.find_first_not_of(' ') == nachricht.npos ) {// Keine Zeichen oder nur Leerzeichen eingegeben
        qWarning("Keine Nachricht eingegeben!");
        return;
    }

    ui.NachrichtB -> setText("");

    if ( nachricht[0] == '/' ) { // Kommando
        boost::tokenizer <sep> tokens ( nachricht, sep(" ") );

        auto tok_it = tokens.begin();
        string const kommando = *tok_it;
        string const& argument { ++tok_it == tokens.end() ? "" : *tok_it };

        if ( kommando == "/exit" )
            flags.set( x_close ); // an pruefen_main() (pruefen.cpp)
        else if ( kommando == "/restart" )
            flags.set( x_restart );
        else if ( kommando == "/reload" )
            flags.set( x_reload ); // an verlauf_up(), aktualisieren.cpp
        else if ( kommando == "/help" )
            hilfe_anz(); // dialog.cpp
        else if ( kommando == "/vordergrund" )
            ui.actionImmer_im_Vordergrund -> toggle();
        else if ( kommando == "/chat" )
            ch_chat( argument ); // privatchats.cpp
        else if ( nutzer_ich.admin && kommando == "/all" ) // Strg+T und /all geht in jedem Chat (als Admin)
            allt(); // kommandos.cpp
        else if ( nutzer_ich.admin && flags[chatall] ) {
            if ( kommando == "/terminate" )
                entfernen( argument ); // dialog.cpp
            else if ( kommando == "/info" )
                info_open( argument );
            else if ( kommando == "/verboten" )
                verbotene_namen_dialog( this );
            else if ( kommando == "/reset" )
                resetcv(); // kommandos.cpp
            else if ( kommando == "/warnung" )
                warnung_send(); // kommandos.cpp
            else if ( kommando == "/drin" )
                nutzer_anz(); // dialog.cpp
            else if ( flags[std_admin] && kommando == "/passwort" )
                openAdminPass();
            else if ( kommando == "/plum" )
                plum_chat(); // kommandos.cpp
            else if ( kommando == "/schreibeinfo" ) // Information in Chatdatei schreiben
                Datei_lock_append( *chatfile_all, chatfile_all_mtx, nachricht.c_str() + kommando.length() + 1 );
            else if ( kommando == "/lock" && flags[x_oberadmin] ) {
                flags.flip( locked ); // lock-Status aufs Gegenteil setzen
                createDialog( "Bestätigung", flags[locked] ? "Keiner darf " OBERADMIN " entfernen!" : "Admins dürfen " OBERADMIN " entfernen!", this );
            }
        }
    } else try { // Nachricht senden
        klog("senden");

        string::const_iterator str_pos = nachricht.cbegin();
        string::const_iterator const str_end = nachricht.cend();
        string::const_iterator line_end;
        bool first = true;

        if ( flags[chatall] ) chatfile_all_mtx.lock();

        std::ofstream chatdatei = chatfile -> ostream( true );

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

        if ( flags[chatall] ) {
            chatdatei.close();
            chatfile_all_mtx.unlock();
        }
    } catch ( fstream_exc const& exc ) {
        ofstreamExcAusgabe( exc, *chatfile );
        klog("Erstelle Chatdatei neu...");
        chatfile -> remove();
        chatfile -> touch();

        if ( flags[chatall] ) chatfile_all_mtx.unlock();
    }
}
