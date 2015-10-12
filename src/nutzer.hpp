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

// Dieser Header deklariert die Klasse Nutzer

#ifndef NUTZER_HPP
#define NUTZER_HPP

#include "ui_nutzer.h"

class Hineinschreiben;

/// Dieser Dialog zeigt alle %Nutzer mit ihrem Benutzernamen_str an. Bei Admins wird ein " (Admin)" angehängt.
class Nutzer : public QDialog
{
    Q_OBJECT

public:
    explicit Nutzer( Hineinschreiben const& nutzer, Hineinschreiben const& admins, QWidget* parent = nullptr ); ///< Konstruktor

private:
    Ui::Nutzer ui {}; ///< UI des Dialogs
};

#endif // NUTZER_HPP
