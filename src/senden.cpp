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
#include "pc_nutzername.hpp"
#include "chatverwaltung.hpp"
#include "klog.hpp"
#include <boost/tokenizer.hpp>

using sep = boost::char_separator <char>;

/// Sendet eine Nachricht oder führt ein eingegebenes Kommando aus.
/**
 * Wird nach Eingabe einer Nachricht aufgerufen und wertet das Eingegebene aus.
 * Wenn die Eingabe mit einem '/' beginnt, wird sie als Kommando interpretiert und die entsprechende Aktion wird ausgeführt.
 * Wenn nicht, wird die Eingabe als Nachricht interpretiert und ChatVerwaltung::schreibeNachricht() wird aufgerufen.
 */
void Chat::senden_pruef() {
    ui.NachrichtB->setFocus(); // Fokus zurück auf Input
    std::string const nachricht = ui.NachrichtB->text().toStdString(); // Eingegebener Text

    if ( nachricht.find_first_not_of(' ') == nachricht.npos ) {// Keine Zeichen oder nur Leerzeichen eingegeben
        qWarning("Keine Nachricht eingegeben!");
        return;
    }

    ui.NachrichtB -> setText("");

    if ( nachricht[0] == '/' ) { // Kommando
        boost::tokenizer <sep> tokens ( nachricht, sep(" ") );

        auto tok_it = tokens.begin();
        std::string const kommando = *tok_it;
        std::string const& argument { ++tok_it == tokens.end() ? "" : *tok_it };

        if ( kommando == "/exit" )
            flags.set( x_close ); // an pruefen_main() (pruefen.cpp)
        else if ( kommando == "/restart" )
            flags.set( x_restart );
        else if ( kommando == "/reload" )
            flags.set( x_reload ); // an verlauf_up(), aktualisieren.cpp
        else if ( kommando == "/help" )
            hilfe_anz(); // dialog.cpp
        else if ( kommando == "/vordergrund" )
            ui.actionImmer_im_Vordergrund->toggle();
        else if ( kommando == "/chat" )
            chat_verwaltung.changeChat( argument );
        else if ( nutzer_ich.admin && kommando == "/all" ) // Strg+T und /all geht in jedem Chat (als Admin)
            allt(); // kommandos.cpp
        else if ( nutzer_ich.admin && chat_verwaltung.imKlassenchat() ) {
            if ( kommando == "/terminate" )
                entfernen( argument ); // dialog.cpp
            else if ( kommando == "/info" )
                info_open( argument );
            else if ( kommando == "/verboten" )
                verbotene_namen_dialog( this );
            else if ( kommando == "/verbotenpc" )
                verbotene_pc_nutzernamen_dialog( this );
            else if ( kommando == "/reset" )
                chat_verwaltung.reset();
            else if ( kommando == "/warnung" )
                warnung_send(); // kommandos.cpp
            else if ( kommando == "/drin" )
                nutzer_anz(); // dialog.cpp
            else if ( flags[std_admin] && kommando == "/passwort" )
                openAdminPass();
            else if ( kommando == "/plum" )
                plum_chat(); // kommandos.cpp
            else if ( kommando == "/schreibeinfo" ) // Information in Chatdatei schreiben
                chat_verwaltung.schreibeInfo( nachricht.c_str() + kommando.length() + 1 );
            else if ( kommando == "/lock" && flags[x_oberadmin] ) {
                flags.flip( locked ); // lock-Status aufs Gegenteil setzen
                createDialog( "Bestätigung", flags[locked] ? "Keiner darf " OBERADMIN " entfernen!" : "Admins dürfen " OBERADMIN " entfernen!", this );
            }
        }
    } else { // Nachricht senden
        klog("senden");
        chat_verwaltung.schreibeNachricht( nachricht );
    }
}
