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

// Dieser Header deklariert die Klasse PersonalO

#ifndef PERSONALO_HPP
#define PERSONALO_HPP

#include "ui_personalo.h"

class Chat;
class Hineinschreiben;

/// Mit diesem Dialog kann man einen neuen Privatchat starten. Es wird Chat::make_chat() mit dem ausgewählten Benutzernamen aufgerufen.
class PersonalO : public QDialog
{
    Q_OBJECT

public:
    explicit PersonalO( std::string const& benutzername, Hineinschreiben const& nutzer, std::string const& name_arg, Chat* parent ); ///< Konstruktor.

    ///\cond
    PersonalO( PersonalO const& ) = delete;
    PersonalO& operator = ( PersonalO const& ) = delete;
    ///\endcond

private:
    Chat* chat_par; ///< Parent
    Ui::PersonalO ui {}; ///< UI des Dialogs

    ///\cond
    void start();
    ///\endcond
};

#endif // PERSONALO_HPP