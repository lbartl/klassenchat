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

// Diese Datei steuert Kommandos

#include "chat.hpp"
#include "simpledialog.hpp"
#include "chatverwaltung.hpp"
#include "filesystem.hpp"
#include <QDebug>

using namespace static_paths;

/// Überall den %Chat beenden (Admin).
void Chat::allt() {
    qDebug("++terminate-all++");
    alltfile.touch();
}

/// Eine Warnung an alle senden (Admin).
void Chat::warnung_send() {
    if ( nutzer_ich.x_plum ) // vom Plum-Chat aus können keine Warnungen gesendet oder empfangen werden
        return;

    qDebug("Warnung senden..");
    lock_guard lock ( pruefen_mtx ); // Damit ich nicht selbst die Warnung bekomme

    warnfile.touch();

    this_thread::sleep_for( 2s ); // 2 Sekunden warten, bis alle die Warnung erkannt haben

    createDialog( "Bestätigung", "Warnung wurde gesendet!", this ); // Bestätigungs-Dialog

    warnfile.remove();
}

/// In den Plum-Chat wechseln oder zurück (Admin).
void Chat::plum_chat() {
    lock_guard l1 ( nutzer_mtx ),
               l2 ( pruefen_mtx );

    if ( nutzer_verwaltung.vorhanden( ! nutzer_ich.x_plum, nutzer_ich.nutzername ) ) {
        createDialog( "Fehler", "Im anderen Chat ist bereits jemand mit diesem Nutzernamen angemeldet!" );
        return;
    }

    lockfile_mtx.lock();
        lockfile->remove();
    lockfile_mtx.unlock();

    nutzer_verwaltung.flip_x_plum();
    chat_verwaltung.flip_x_plum();

    if ( nutzer_ich.x_plum ) { // Kommt vom normalen Chat
        lockfile = &lockfile_plum;
        ui.actionIn_den_Plum_Chat_wechseln->setText("&In den normalen Chat wechseln");
   } else { // Kommt vom Plum-Chat
        lockfile = &lockfile_norm;
        ui.actionIn_den_Plum_Chat_wechseln->setText("&In den Plum-Chat wechseln");
    }

    ui.actionWarnung_senden->setEnabled( ! nutzer_ich.x_plum );

    if ( ! lockfile->exist() )
        if ( this_thread::sleep_for( 200ms ), ! lockfile->exist() ) // siehe start.cpp, lockfile_exist()
            chat_verwaltung.reset();

    lockfile_mtx = Datei_Mutex( *lockfile );
    checkfile = makeToNutzerDatei( checkdir, nutzer_ich );
    terminatefile = makeToNutzerDatei( terminatedir, nutzer_ich );
    infofile = makeToNutzerDatei( infodir, nutzer_ich );

    terminatefile.remove();
    infofile.remove();
}

/// Ein neues %Passwort für mich setzen (#std_admin).
/**
 * @param newpass neues %Passwort
 *
 * Ruft AdminPass::setpass() auf.
 * Wenn das neue %Passwort ungültig ist, wird ein Dialog angezeigt.
 */
void Chat::set_pass( std::string newpass ) try {
    passwords.setpass( std::move( newpass ) );
} catch ( std::invalid_argument const& exc ) {
    createDialog( "Fehler", exc.what(), this, true );
    openAdminPass();
}
