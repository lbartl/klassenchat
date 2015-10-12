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

// Diese Datei definiert die Klasse Passwort

#include "passwort.hpp"
#include <QPushButton>

Passwort::Passwort( QWidget* parent ) :
    QDialog( parent )
{
    ui.setupUi( this );
    ui.buttonBox -> button( QDialogButtonBox::Cancel ) -> setText("Abbrechen");

    this -> setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    ui.PasswortA -> setFocus(); // Fokus auf Input

    connect( this, &Passwort::accepted, [this] () { setzen(); } );
}

void Passwort::setzen() { // Passwort setzen und Signal aussenden
    passein = ui.PasswortA -> text().toStdString();
    emit eingegeben( passein );
}
