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

// Diese Datei definiert die Klasse Warnung

#include "warnung.hpp"
#include "chat.hpp"

///\cond
Warnung::Warnung( Chat* parent ) :
    QDialog( parent )
{
    ui.setupUi( this );
    ui.label -> setText("Bitte den Chat verlassen!\n\nWenn du dennoch im Chat bleiben willst, kannst du den Chat mit Strg + Q rechtzeitig beenden!\nBesser wäre aber du würdest jetzt rausgehen!");

    this -> setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    ui.ButtonV -> setFocus();

    connect( ui.ButtonB, &QPushButton::clicked, this, &Warnung::close ); // Im Chat bleiben, nur Warnung schließen
    connect( ui.ButtonV, &QPushButton::clicked, parent, &Chat::close );  // Chat beenden, ganzen Chat schließen
}
///\endcond
