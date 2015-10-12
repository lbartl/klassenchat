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

// Dieser Header deklariert die Klasse Entfernen

#ifndef ENTFERNEN_HPP
#define ENTFERNEN_HPP

#include "ui_entfernen.h"

class Hineinschreiben;

/// Mit diesem Dialog kann ein %Admin einen %Nutzer entfernen.
class Entfernen : public QDialog
{
    Q_OBJECT

public:
    explicit Entfernen( QString const& benutzername_str, Hineinschreiben const& nutzer, std::string const& ter_name = "", QWidget* parent = nullptr ); ///< Konstruktor.

private:
    Ui::Entfernen ui {}; ///< UI des Dialogs
    QString const& nutzername_str; ///< Chat::nutzername_str

    ///\cond
    void schreiben() const;
    ///\endcond
};

#endif // ENTFERNEN_HPP
