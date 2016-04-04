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

// Dieser Header deklariert die Klassen Admin_anz und Nutzer_anz

#ifndef NUTZER_ANZ_HPP
#define NUTZER_ANZ_HPP

#include "ui_admin_anz.h"
#include "ui_nutzer_anz.h"
#include <vector>

class AdminPass;

/// Dieser Dialog zeigt alle %Nutzer mit ihrem Benutzernamen_str an. Bei Admins wird ein " (Admin)" angehängt.
class Nutzer_anz : public QDialog {
    Q_OBJECT

public:
    explicit Nutzer_anz( QWidget* parent = nullptr ); ///< Konstruktor

private:
    Ui::Nutzer_anz ui {}; ///< UI des Dialogs
};

/// Dieser Dialog ist nur für den Chat::oberadmin
/**
 * Dieser Dialog zeigt alle %Nutzer mit ihrem Benutzername_str an und (falls sie einer der Chat::std_admins sind) mit ihrem %Passwort.
 * Sie können dann zum %Admin gemacht werden oder entmachtet werden.
 */
class Admin_anz : public QDialog {
    Q_OBJECT

public:
    explicit Admin_anz( AdminPass const& admin_pass, QWidget* parent = nullptr ); ///< Konstruktor

private:
    Ui::Admin_anz ui {}; ///< UI des Dialogs
    std::vector <std::pair<size_t, QListWidgetItem>> anz_nutzer {}; ///< Alle angezeigten Nutzer

    ///\cond
    void schreiben() const;
    ///\endcond
};

#endif // NUTZER_ANZ_HPP
