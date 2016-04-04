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

// Dieser Header deklariert die Klasse InfoOpen

#ifndef INFOOPEN_HPP
#define INFOOPEN_HPP

#include "ui_infoopen.h"

class Hineinschreiben;

/// Mit diesem Dialog kann ein %Admin Informationen versenden.
class InfoOpen : public QDialog
{
    Q_OBJECT

public:
    explicit InfoOpen( std::string const& an = "", QWidget* parent = nullptr ); ///< Konstruktor.

private:
    Ui::InfoOpen ui {}; ///< UI des Dialogs

    ///\cond
    void schreiben() const;
    ///\endcond
};

#endif // INFOOPEN_HPP
