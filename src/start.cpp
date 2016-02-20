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

// Diese Datei steuert den Start und das Beenden des Chats

#include "chat.hpp"
#include "lockfile.hpp"
#include "filesystem.hpp"
#include "klog.hpp"
#include <QTimer>

using namespace static_paths;
using std::make_unique;

inline bool lockfile_exist( Datei const& lockfile ) {
    if ( lockfile.exist() )
        return true;
    else {
        this_thread::sleep_for( 0.2s ); // Es könnte sein, dass währenddessen jemand das Lockfile erstellt
        return lockfile.exist();
    }
}

/// Starten des Chats. Aufgerufen nach dem Anmelden.
/**
 * 1. Setzt meinen Benutzernamen auf den eingegebenen Namen.
 * 2. Ruft vergeben() auf.
 * 3. Prüft ob mein Benutzername in #std_admins vorhanden ist, und setzt danach #std_admin und #admin.
 * 4. Weißt #passwords zu.
 * 5. Wenn ich ein #std_admin bin, wird das Passwort-Feld geöffnet, wenn nicht, wird start2() aufgerufen.
 */
void Chat::start() {
    nutzername = ui.NutzernameA -> text().toStdString(); // mein Benutzername
    checkfile = makeToNutzerDatei( checkdir, x_plum, nutzername );

    if ( lockfile_exist( *lockfile ) && vergeben() ) { // Es ist bereits jemand mit diesem Benutzernamen angemeldet (pruefen.cpp)
        klog("Nochmal Benutzername eingeben...");
        ui.NutzernameA -> setFocus();
        return;
    }

    nutzername_str = toBenutzername_str( x_plum, nutzername );
    bool const is_oberadmin = ! x_plum && nutzername == oberadmin;
    bool const is_std_admin = is_oberadmin || ( ! x_plum && enthaelt( std_admins.begin(), std_admins.end(), nutzername ) );
    flags[x_oberadmin] = is_oberadmin; // Oberadmin
    flags[std_admin] = is_std_admin; // Standard-Admin
    flags[admin] = is_std_admin; // Admin
    ui.action_berall_den_Chat_beenden -> setEnabled( is_std_admin ); // Explizit abschalten, da Tastenkombination
    ui.actionNeues_Admin_Passwort -> setEnabled( is_std_admin );
    ui.actionWarnung_senden -> setEnabled( ! x_plum );

    if ( is_std_admin ) {
        passwords = make_unique <AdminPass> ( nutzername );

        klog("Admin, öffne Passwortfeld...");

        ui.Label2 -> setText( "Passwort für " + nutzername_str + ": " ); // std_admin nur im normalen Chat, deswegen ist nutzername_str hier gleich wie nutzername
        ui.LineStackedWidget -> setCurrentIndex( 1 ); // Passwort
    } else
        start2();
}

/// Überprüft das %Passwort. Aufgerufen nach Eingabe des Passworts.
/**
 * Wenn das eingegebene %Passwort richtig war, wird start2() aufgerufen, wenn nicht, wird der %Chat beendet.
 */
void Chat::passwort() {
    if ( passwords -> getpass() == ui.PasswortA -> text().toStdString() ) { // richtiges Passwort
        ui.PasswortA -> setText(""); // für einen Neustart
        ui.LineStackedWidget -> setCurrentIndex( 0 ); // für einen Neustart
        start2();
    } else { // Falsches Passwort, beenden
        qCritical("Falsches Admin-Passwort!");
        qApp -> exit( EXIT_FAILURE );
    }
}

/// Starten des Chats. Aufgerufen durch start() oder passwort().
/**
 * 1. Überprüft ob jemand im %Chat ist, wenn nicht lösche ich den Chatverlauf (#admin) oder verlasse den %Chat (Lockfile).
 * 2. Ruft setfiles(), klassenchat() und start_threads() auf.
 * 4. Öffnet das Chatfenster.
 * 5. Schreibt in #chatfile_all, dass ich den Chat betreten habe.
 * 6. Ruft verlauf_up() auf.
 */
void Chat::start2() { // Nachdem Admin Passwort eingegeben hat
    bool const lock = lockfile_exist( *lockfile ),
               darf = flags[admin] || x_plum;

    if ( darf && ! lock ) { // Admin oder Plum-Chat, Chatverlauf löschen
        resetcv();

        Datei const& lockfile_other { x_plum ? lockfile_norm : lockfile_plum }; // Lockfile des anderen Chats

        if ( ! lockfile_exist( lockfile_other ) ) {
            nutzerfile.reset();
            adminfile.reset();

            senddir.removeInhalt();
            infodir.removeInhalt();
            terminatedir.removeInhalt();
            checkdir.removeInhalt();
        }

        lockfile -> touch();
    } else if ( ! darf && ! lock ) { // kein Admin, Dialog um den Chat zu beenden
        klog("Kein Admin im Chat!");
        Lockfile( this ).exec();
        return;
    }

    setfiles();
    klassenchat();
    start_threads();

    this -> setWindowTitle( QString::fromStdString( nutzername + " - Chat" ) );

    ui.menuBar -> setVisible( true ); // Menüleiste einblenden

    ui.NachrichtB -> setFocus(); // Fokus auf Input
    ui.MainStackedWidget -> setCurrentIndex( 1 ); // Chatfenster

    Datei_append( *chatfile_all, chatfile_all_mtx, nutzername + " hat den Chat betreten" );
    flags.set( x_main ); // Info an closeEvent, dass sich das Fenster nicht mehr einfach schließen lässt, sondern x_close gesetzt werden soll (chat.cpp)
    main_thread(); // aktualisieren.cpp
}

/// Wird aufgerufen, nachdem #nutzername und #x_plum oder #nutzername_str gesetzt oder verändert wurden.
/**
 * Zuweisung von #chatfile_all_mtx, #nutzer_h, #admins_h, #terminatefile und #infofile.
 * Diese benötigen entweder den #nutzername%n und #x_plum oder #nutzername_str.
 */
void Chat::setfiles() {
    nutzer_h = make_unique <Hineinschreiben> ( nutzername_str, nutzerfile );
    if ( flags[admin] ) admins_h = make_unique <Hineinschreiben> ( nutzername_str, adminfile );

    terminatefile = makeToNutzerDatei( terminatedir, x_plum, nutzername );
    infofile = makeToNutzerDatei( infodir, x_plum, nutzername );

    terminatefile.remove();
    infofile.remove();
}

/// %Chat beenden oder neustarten.
/**
 * Beendet den %Chat oder startet ihn neu, je nachdem ob #x_restart gesetzt wurde.
 * Wird von verlauf_up() aufgerufen, wenn pruefen_main() true zurückgibt.
 *
 * Schließt zuerst das Fenster und ruft dann stop_threads() auf.
 * Bei einem Neustart wird anschließend alles auf den Zustand nach dem Konstruktor-Aufruf gesetzt und dann das Fenster wieder angezeigt.
 */
void Chat::stop() {
    flags.reset( x_main );
    this -> close(); // Fenster schließen

    stop_threads();

    if ( flags[x_restart] ) { // Chat neustarten
        ui.BrowseA -> setText("");
        ui.MainStackedWidget -> setCurrentIndex( 0 );
        ui.NutzernameA -> selectAll();
        ui.menuBar -> setVisible( false );

        flags.reset();
        admins_h.reset();
        passwords.reset();

        this -> setWindowTitle("Handout Kräuterhexe");
        this -> show();
    } else
        qApp -> quit(); // Chat beenden
}
