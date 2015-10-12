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

// Dieser Header deklariert die Klasse Admin

#ifndef ADMIN_HPP
#define ADMIN_HPP

#include "ui_admin.h"
#include "hineinschreiben.hpp"

class AdminPass;

/// Dieser Dialog ist nur für den Chat::oberadmin
/**
 * Dieser Dialog zeigt alle %Nutzer mit ihrem Benutzername_str an und (falls sie einer der Chat::std_admins sind) mit ihrem %Passwort.
 * Sie können dann zum %Admin gemacht werden oder entmachtet werden.
 */
class Admin : public QDialog
{
    Q_OBJECT

public:
    explicit Admin( Hineinschreiben const& nutzer, Hineinschreiben const& admins, AdminPass const& admin_pass, QWidget* parent = nullptr ); ///< Konstruktor

private:
    Ui::Admin ui {}; ///< UI des Dialogs
    std::vector <QString> const admins_v; ///< Hineinschreiben kann wegen Datei_Mutex nicht kopiert werden

    ///\cond
    void schreiben() const;
    ///\endcond
};

#endif // ADMIN_HPP
