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

// Diese Datei steuert die Erstellung und Zerstörung des Hauptfensters

#include "chat.hpp"
#include "filesystem.hpp"
#include "klog.hpp"
#include <QCloseEvent>

// Verwalter
#ifndef VERWALTER
# define VERWALTER "Lukas Bartl"
#endif

// Versions Makros
#define VERSION "1.5.1" // Versions-Nummer
#define BUILD   "0054"  // Build-Nummer
#define TYPE    "alpha"  // Build-Typ

// statische Member definieren
///\cond
constexpr decltype( Chat::std_admins ) Chat::std_admins;
///\endcond

Datei const Chat::chatfile_norm = "./verlauf.jpg", ///< Datei, in der der normale %Chat gespeichert ist
            Chat::chatfile_plum = "./baum.jpg", ///< Datei, in der der Plum-Chat gespeichert ist
            Chat::lockfile_norm = "./lock", ///< Datei, die existiert, wenn jemand im normalen %Chat ist
            Chat::lockfile_plum = "./baum"; ///< Datei, die existiert, wenn jemand im Plum-Chat ist

/**
 * @param plum ob ich im Plum-Chat bin
 * @param parent Eltern-Objekt
 */
Chat::Chat( bool plum, QWidget* parent ) :
    QMainWindow( parent ),
    x_plum( plum ),
    chatfile_all( plum ? &chatfile_plum : &chatfile_norm ),
    lockfile( plum ? &lockfile_plum : &lockfile_norm )
{
    ui.setupUi( this );
    ui_sep = ui.menuChats -> insertSeparator( ui.actionNeuer_Chat );

    ui.menuBar -> setVisible( false ); // Menüleiste ausblenden
    ui.MainStackedWidget -> setCurrentIndex( 0 ); // Anmelde-Fenster
    ui.LineStackedWidget -> setCurrentIndex( 0 ); // Nutzername

    ui.NutzernameA -> setValidator( new QRegExpValidator( QRegExp("[\\wÄäÖöÜüß_]+"), ui.NutzernameA ) ); // Nur Buchstaben und Zahlen erlauben, keine Sonderzeichen und Leerzeichen
    ui.NutzernameA -> setMaxLength( 20 );

    if ( plum ) { // Plum-Chat
        ui.Copyright -> setText("Chat");

        ui.actionIn_den_Plum_Chat_wechseln -> setText("&In den normalen Chat wechseln");
    } else // Normaler Chat
        ui.Copyright -> setText( "Copyright (C) 2015 Lukas Bartl\n"
                                 "Lizenz GPLv3+: GNU GPL Version 3 oder höher <http://gnu.org/licenses/gpl.html>\n"
                                 "Dieses Programm ist freie Software. Es darf verändert und weitergegeben werden.\n"
                                 "Es gibt keinerlei Garantien.\n\n"
                                 "Den Quellcode gibt es auf https://github.com/hanswurst862/klassenchat\n\n"
                                 "Wenn jemand Verbesserungsvorschläge hat, kann er sich bei " VERWALTER " melden!\n\n"
                                 "Version: " VERSION "-" BUILD " " TYPE );

    ui.Copyright -> setTextInteractionFlags( Qt::TextSelectableByMouse ); // Text kann von Nutzer ausgewählt werden

    connect( ui.NutzernameA, &QLineEdit::returnPressed, ui.Ok1,    &QPushButton::click );
    connect( ui.PasswortA,   &QLineEdit::returnPressed, ui.Ok2,    &QPushButton::click );
    connect( ui.NachrichtB,  &QLineEdit::returnPressed, ui.Senden, &QPushButton::click );

    connect( ui.Ok1,    &QPushButton::clicked, [this] () { start();        } );
    connect( ui.Ok2,    &QPushButton::clicked, [this] () { passwort();     } );
    connect( ui.Senden, &QPushButton::clicked, [this] () { senden_pruef(); } );

    connect( ui.actionQuit, &QAction::triggered, this, &Chat::close ); // closeEvent aufrufen

    connect( ui.actionImmer_im_Vordergrund, &QAction::toggled, this, &Chat::vordergrund );

    connect( ui.actionHilfe,       &QAction::triggered, [this] () { hilfe_anz();   } );
    connect( ui.actionKlassenchat, &QAction::triggered, [this] () { klassenchat(); } );
    connect( ui.actionNeuer_Chat,  &QAction::triggered, [this] () { personal_op(); } );
    connect( ui.actionNeue_Gruppe, &QAction::triggered, [this] () { group_open();  } );

    connect( ui.actionEinen_Nutzer_entfernen,       &QAction::triggered, [this] () { entfernen();     } );
    connect( ui.actionChatverlauf_l_schen,          &QAction::triggered, [this] () { resetcv();       } );
    connect( ui.action_berall_den_Chat_beenden,     &QAction::triggered, [this] () { allt();          } );
    connect( ui.actionWarnung_senden,               &QAction::triggered, [this] () { warnung_send();  } );
    connect( ui.actionWer_ist_alles_im_Chat,        &QAction::triggered, [this] () { nutzer_anz();    } );
    connect( ui.actionIn_den_Plum_Chat_wechseln,    &QAction::triggered, [this] () { plum_chat();     } );
    connect( ui.actionInformation_an_Nutzer_senden, &QAction::triggered, [this] () { info_open();     } );
    connect( ui.actionVerbotene_Benutzernamen,      &QAction::triggered, [this] () { ver_open();      } );
    connect( ui.actionNeues_Admin_Passwort,         &QAction::triggered, [this] () { openAdminPass(); } );

#ifndef _DEBUG
    delete ui.actionNeue_Gruppe;
#endif
}

void Chat::closeEvent( QCloseEvent* event ) {
    event -> ignore();

    if ( flags[x_main] ) // Im Chat-Fenster
        flags.set( x_close ); // an pruefen_main() (pruefen.cpp)
    else // Noch nicht im Chat-Fenster
        event -> accept(); // Fenster schließen
}

/// Fenster immer im Vordergrund anzeigen
/**
 * @param status true für immer im Vordergrund, false für nicht immer im Vordergrund.
 *
 * Dieser Slot ist mit der QAction "Immer im Vordergrund" verbunden.
 */
void Chat::vordergrund( bool const status ) { // Chatfenster immer im Vordergrund anzeigen
    this -> setWindowFlags( status ? windowFlags() | Qt::WindowStaysOnTopHint : windowFlags() & ~Qt::WindowStaysOnTopHint );

    this_thread::sleep_for( 0.1s ); // 0,1 Sekunden warten bis Window Manager Flag erkannt hat

    this -> show();
}
