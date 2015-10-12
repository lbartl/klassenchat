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

// Diese Datei definiert die Klasse Lockfile

#include "lockfile.hpp"
#include <QCloseEvent>

Lockfile::Lockfile( QWidget* parent ) :
    QDialog( parent )
{
    ui.setupUi( this );
    ui.label -> setText("Momentan ist kein Administrator im Chat!\n\nDas Benutzen des Chats ist nur möglich,\nwenn ein Administrator im Chat ist!");

    this -> setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    connect( ui.ButtonBeenden, &QPushButton::clicked, this, &Lockfile::close ); // closeEvent aufrufen
}

void Lockfile::closeEvent( QCloseEvent* event ) {
    event -> ignore();
    qApp -> exit( EXIT_FAILURE ); // Programm beenden
}
