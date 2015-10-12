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

// Diese Datei definiert die Klasse SimpleDialog und die Funktion createDialog

#include "simpledialog.hpp"

SimpleDialog::SimpleDialog( QString const& titel, QString const& text, QWidget* parent ) :
    QDialog( parent )
{
    ui.setupUi( this );

    this -> setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    this -> resize( titel.length() * 5 + 150, height() ); // Das 5-fache der Länge des Titels plus 150 als Richtwert, damit Titel auch vollständig angezeigt wird

    this -> setWindowTitle( titel );
    ui.label -> setText( text );
}

void createDialog( QString const& titel, QString const& text, QWidget* parent, bool modal ) { // einen einfachen Dialog machen
    SimpleDialog*const dialog = new SimpleDialog( titel, text, parent );

    if ( modal ) dialog -> setWindowModality( Qt::ApplicationModal );

    dialog -> setAttribute( Qt::WA_DeleteOnClose );
    dialog -> show();
}
