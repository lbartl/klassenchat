/* Copyright (C) 2016 Lukas Bartl
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

///\file
// Dieser Header definiert die Klasse Ueber

#ifndef UEBER_HPP
#define UEBER_HPP

#include "simpledialog.hpp"

///\cond
// Verwalter
#ifndef VERWALTER
# define VERWALTER "Lukas Bartl"
#endif

// Versions Makros
#define VERSION "1.8.1"  // Versions-Nummer
#define BUILD   "0084"   // Build-Nummer
#define TYPE    "alpha"  // Build-Typ
///\endcond

/// Wird auf Startseite und unter "Über..." angezeigt
#define UEBER_KURZTEXT "Copyright (C) 2015-2016 Lukas Bartl<br>" \
"Dieses Programm ist freie Software. Es darf verändert und weitergegeben werden.<br>" \
"Es gibt keinerlei Garantien.<br>" \
"Lizenz <a href='https://www.gnu.org/licenses/gpl.html'>GPLv3+</a>: GNU GPL Version 3 oder höher<br><br>" \
"Den Quellcode gibt es auf " \
"<a href='https://www.github.com/hanswurst862/klassenchat'>github.com/hanswurst862/klassenchat</a><br><br>" \
"Wenn jemand Verbesserungsvorschläge hat, kann er sich bei " VERWALTER " melden!<br><br>" \
"Version: " VERSION "-" BUILD " " TYPE

/// Wird unter "Über..." angezeigt
#define UEBER_TEXT UEBER_KURZTEXT "<br><br>" \
"Verwendete Bibliotheken:<br>" \
"<a href='https://www.qt.io/'>Qt5</a> " \
"(<a href='https://doc.qt.io/qt-5/lgpl.html'>Lizenz</a>)<br>" \
"<a href='www.boost.org/'>boost</a> " \
"(<a href='www.boost.org/users/license.html'>Lizenz</a>)<br>" \
"<a href='https://www.github.com/meganz/mingw-std-threads/tree/master'>mingw-std-threads</a> " \
"(<a href='https://www.github.com/meganz/mingw-std-threads/blob/master/LICENSE'>Lizenz</a>)"

/// Dieser Dialog zeigt Informationen über den %Chat an. Er wird mit der Funktion ueber_dialog() erstellt.
class Ueber : public SimpleDialog
{
    Q_OBJECT

public:
    static constexpr char const* kurztext = UEBER_KURZTEXT; ///< Für andere verwendbar

    /// Konstruktor.
    /**
     * @param parent Parent
     */
    Ueber( QWidget* parent = nullptr ) :
        SimpleDialog( "Über", UEBER_TEXT, parent )
    {
        ui.label->setTextFormat( Qt::RichText );
        ui.label->setTextInteractionFlags( Qt::TextBrowserInteraction ); // Links können von Nutzer angeklickt werden
        ui.label->setOpenExternalLinks( true ); // Links werden in Browser geöffnet
    }
};

#undef UEBER_TEXT
#undef UEBER_KURZTEXT

/// Erstellt ein Objekt der Klasse Ueber.
/**
 * @param parent Parent
 */
inline void ueber_dialog( QWidget* parent = nullptr ) {
    Ueber*const dialog = new Ueber( parent );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->show();
}

#endif // UEBER_HPP
