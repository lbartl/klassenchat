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

// Diese Datei steuert Kommandos

#include "chat.hpp"
#include "simpledialog.hpp"
#include "filesystem.hpp"
#include "klog.hpp"

using namespace static_paths;
using std::string;

/// Chatverlauf löschen (#admin).
void Chat::resetcv() {
    klog("reset");

    chatfile_all_mtx.lock();
        chatfile_all -> reset();
    chatfile_all_mtx.unlock();

    alltfile.remove();

    if ( ! x_plum || flags[x_main] ) {
        warnfile.remove();
        terminatedir.removeInhalt();
    }
}

/// Überall den %Chat beenden (#admin).
void Chat::allt() {
    klog("++terminate-all++");
    alltfile.touch();
}

/// Eine Warnung an alle senden (#admin).
void Chat::warnung_send() {
    if ( x_plum ) // vom Plum-Chat aus können keine Warnungen gesendet oder empfangen werden
        return;

    klog("Warnung senden..");
    lock_guard lock ( pruefen_mtx ); // Damit ich nicht selbst die Warnung bekomme

    warnfile.touch();

    this_thread::sleep_for( 2s ); // 2 Sekunden warten, bis alle die Warnung erkannt haben

    createDialog( "Bestätigung", "Warnung wurde gesendet!", this ); // Bestätigungs-Dialog

    warnfile.remove();
}

/// In den Plum-Chat wechseln oder zurück (#admin).
void Chat::plum_chat() {
    lock_guard l1 ( nutzer_mtx ),
               l2 ( pruefen_mtx );

    x_plum = ! x_plum; // flippen

    if ( vergeben() ) { // Im anderen Chat ist jemand mit meinem Namen angemeldet
        x_plum = ! x_plum;
        return;
    }

    lockfile_mtx.lock();
        lockfile -> remove();
    lockfile_mtx.unlock();

    nutzer_h -> herausnehmen();
    admins_h -> herausnehmen();

    nutzername_str = toBenutzername_str( x_plum, nutzername );

    string normtext = nutzername,
           plumtext = nutzername;

    if ( x_plum ) { // Kommt vom normalen Chat
        chatfile_all = &chatfile_plum;
        lockfile     = &lockfile_plum;

        plumtext += " hat den Chat betreten";
        normtext += " hat in den Plum-Chat gewechselt";

        ui.actionIn_den_Plum_Chat_wechseln -> setText("&In den normalen Chat wechseln");
   } else { // Kommt vom Plum-Chat
        chatfile_all = &chatfile_norm;
        lockfile     = &lockfile_norm;

        plumtext += " hat den Chat verlassen";
        normtext += " hat in diesen Chat gewechselt";

        ui.actionIn_den_Plum_Chat_wechseln -> setText("&In den Plum-Chat wechseln");
    }

    ui.actionWarnung_senden -> setEnabled( ! x_plum );
    chats_ac.clear(); // alle Privatchats löschen (Chatdateien werden von Partnern gelöscht)

    if ( ! lockfile -> exist() )
        if ( this_thread::sleep_for( 0.2s ), ! lockfile -> exist() ) { // siehe start.cpp, lockfile_exist()
            resetcv();
            lockfile -> touch();
        }

    Datei_Mutex other_file_mtx = std::move( chatfile_all_mtx ); // Datei_Mutex für chatfile_all des anderen Chats
    chatfile_all_mtx = *chatfile_all;

    Datei_lock_append( chatfile_plum, x_plum ? chatfile_all_mtx : other_file_mtx, plumtext );
    Datei_lock_append( chatfile_norm, x_plum ? other_file_mtx : chatfile_all_mtx, normtext );

    setfiles(); // start.cpp

    nutzer_h -> reinschreiben();
    admins_h -> reinschreiben();

    klassenchat();
}

/// Ein neues %Passwort für mich setzen (#std_admin).
/**
 * @param newpass neues %Passwort
 *
 * Ruft AdminPass::setpass auf.
 * Wenn das neue %Passwort ungültig ist, wird ein Dialog angezeigt.
 */
void Chat::set_pass( string newpass ) try {
    passwords -> setpass( std::move( newpass ) );
} catch ( std::invalid_argument const& exc ) {
    createDialog( "Fehler", exc.what(), this, true );
    openAdminPass();
}
