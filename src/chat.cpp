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

// Diese Datei steuert die Erstellung und Zerstörung des Hauptfensters

#include "chat.hpp"
#include "chatverwaltung.hpp"
#include "pc_nutzername.hpp"
#include "global.hpp"
#include <QCloseEvent>

// Verwalter
#ifndef VERWALTER
# define VERWALTER "Lukas Bartl"
#endif

// Versions Makros
#define VERSION "1.7" // Versions-Nummer
#define BUILD   "0073"  // Build-Nummer
#define TYPE    "rc1"  // Build-Typ

// statische Member definieren
///\cond
constexpr decltype( Chat::std_admins ) Chat::std_admins;
///\endcond

Datei const Chat::lockfile_norm {"./lock"}, ///< %Datei, die existiert, wenn jemand im normalen %Chat ist
            Chat::lockfile_plum {"./baum"}; ///< %Datei, die existiert, wenn jemand im Plum-Chat ist

/**
 * @param x_plum ob ich im Plum-Chat bin
 * @param parent Eltern-Objekt
 */
Chat::Chat( bool const x_plum, QWidget* parent ) :
    QMainWindow( parent ),
    x_plum_anfang( x_plum ),
    lockfile( x_plum ? &lockfile_plum : &lockfile_norm )
{
    ui.setupUi( this );

    ui.menuBar->setVisible( false ); // Menüleiste ausblenden
    ui.MainStackedWidget->setCurrentIndex( 0 ); // Anmelde-Fenster
    ui.LineStackedWidget->setCurrentIndex( 0 ); // Nutzername

    ui.NutzernameA->setValidator( new QRegExpValidator( regex_nutzername, ui.NutzernameA ) ); // Nur Buchstaben und Zahlen erlauben, keine Sonderzeichen und Leerzeichen
    ui.NutzernameA->setMaxLength( 20 );
    ui.NutzernameA->setFocus();

    if ( x_plum ) { // Plum-Chat
        ui.Copyright->setText("Chat");
        ui.actionIn_den_Plum_Chat_wechseln->setText("&In den normalen Chat wechseln");
    } else // Normaler Chat
        ui.Copyright->setText( "Copyright (C) 2015-2016 Lukas Bartl<br>"
                               "Dieses Programm ist freie Software. Es darf verändert und weitergegeben werden.<br>"
                               "Es gibt keinerlei Garantien.<br>"
                               "Lizenz GPLv3+: GNU GPL Version 3 oder höher: "
                               "<a href='https://www.gnu.org/licenses/gpl.html'>www.gnu.org/licenses/gpl.html</a><br><br>"
                               "Den Quellcode gibt es auf "
                               "<a href='https://www.github.com/hanswurst862/klassenchat'>www.github.com/hanswurst862/klassenchat</a><br><br>"
                               "Wenn jemand Verbesserungsvorschläge hat, kann er sich bei " VERWALTER " melden!<br><br>"
                               "Version: " VERSION "-" BUILD " " TYPE );

    ui.Copyright->setTextFormat( Qt::RichText );
    ui.Copyright->setTextInteractionFlags( Qt::TextBrowserInteraction ); // Links können von Nutzer angeklickt werden
    ui.Copyright->setOpenExternalLinks( true ); // Links werden in Browser geöffnet

    connect( ui.NutzernameA, &QLineEdit::returnPressed, ui.Ok1,    &QPushButton::click );
    connect( ui.PasswortA,   &QLineEdit::returnPressed, ui.Ok2,    &QPushButton::click );
    connect( ui.NachrichtB,  &QLineEdit::returnPressed, ui.Senden, &QPushButton::click );

    connect( ui.Ok1,    &QPushButton::clicked, [this] () { start();        } );
    connect( ui.Ok2,    &QPushButton::clicked, [this] () { passwort();     } );
    connect( ui.Senden, &QPushButton::clicked, [this] () { senden_pruef(); } );

    connect( ui.actionQuit, &QAction::triggered, this, &Chat::close ); // closeEvent aufrufen
    connect( ui.actionImmer_im_Vordergrund, &QAction::toggled, this, &Chat::vordergrund );

    connect( ui.actionKlassenchat,         &QAction::triggered, [] () { chat_verwaltung.klassenchat(); } );
    connect( ui.actionChatverlauf_l_schen, &QAction::triggered, [] () { chat_verwaltung.reset();       } );

    connect( ui.actionHilfe,                        &QAction::triggered, [this] () { hilfe_anz();     } );
    connect( ui.actionNeuer_Chat,                   &QAction::triggered, [this] () { personal_op();   } );
    connect( ui.actionEinen_Nutzer_entfernen,       &QAction::triggered, [this] () { entfernen();     } );
    connect( ui.action_berall_den_Chat_beenden,     &QAction::triggered, [this] () { allt();          } );
    connect( ui.actionWarnung_senden,               &QAction::triggered, [this] () { warnung_send();  } );
    connect( ui.actionWer_ist_alles_im_Chat,        &QAction::triggered, [this] () { nutzer_anz();    } );
    connect( ui.actionIn_den_Plum_Chat_wechseln,    &QAction::triggered, [this] () { plum_chat();     } );
    connect( ui.actionInformation_an_Nutzer_senden, &QAction::triggered, [this] () { info_open();     } );
    connect( ui.actionNeues_Admin_Passwort,         &QAction::triggered, [this] () { openAdminPass(); } );

    connect( ui.actionVerbotene_Benutzernamen, &QAction::triggered, [this] () { verbotene_namen_dialog( this ); } );
    connect( ui.actionVerbotene_Pc_Nutzernamen, &QAction::triggered, [this] () { verbotene_pc_nutzernamen_dialog( this ); } );
}

void Chat::closeEvent( QCloseEvent* event ) {
    event->ignore();

    if ( flags[x_main] ) // Im Chat-Fenster
        flags.set( x_close ); // an pruefen_main() (pruefen.cpp)
    else // Nicht im Chat-Fenster
        event->accept(); // Fenster schließen
}

/// Fenster immer im Vordergrund anzeigen
/**
 * @param status true für immer im Vordergrund, false für nicht immer im Vordergrund.
 *
 * Dieser Slot ist mit der QAction "Immer im Vordergrund" verbunden.
 */
void Chat::vordergrund( bool const status ) { // Chatfenster immer im Vordergrund anzeigen
    this->setWindowFlags( status ? windowFlags() | Qt::WindowStaysOnTopHint : windowFlags() & ~Qt::WindowStaysOnTopHint );

    this_thread::sleep_for( 100ms ); // 0,1 Sekunden warten bis Window Manager Flag erkannt hat

    this->show();
}
