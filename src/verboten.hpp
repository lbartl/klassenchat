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

// Dieser Header deklariert die Klasse Verboten

#ifndef VERBOTEN_HPP
#define VERBOTEN_HPP

#include "ui_verboten.h"
#include "chat.hpp"

/// Dieser Dialog zeigt alle verbotenen Benutzernamen an.
/**
 * Es können Benutzernamen hinzugefügt werden, indem das #itemneu bearbeitet wird.
 * Es können Benutzernamen entfernt werden, indem der Text gelöscht wird.
 * Es können Benutzernamen verändert werden, indem der Text bearbeitet wird.
 */
class Verboten : public QDialog
{
    Q_OBJECT

public:
    explicit Verboten( QWidget* parent = nullptr ); ///< Konstruktor.

    ///\cond
    Verboten( Verboten const& ) = delete;
    Verboten& operator = ( Verboten const& ) = delete;
    ///\endcond

private:
    Ui::Verboten ui {}; ///< UI des Dialogs
    QListWidgetItem* itemneu {}; ///< Hat immer den Text #neu_text und wird neu erstellt, wenn Text bearbeitet wurde
    unsigned int count {}; ///< Anzahl an Items (ohne #itemneu)
    int const& neupos { reinterpret_cast <int const&> ( count ) }; ///< Position des #itemneu

    static QString const neu_text; ///< Text des #itemneu
    static std::array <QString, Chat::std_admins.size()> std_admins; ///< Chat::std_admins mit QString

    void create_itemneu(); ///< #itemneu erstellen

    ///\cond
    void schreiben();
    ///\endcond

private slots:
    void aktualisieren( QListWidgetItem* item ); ///< Wird aufgerufen, wenn ein Item verändert wurde
};

#endif // VERBOTEN_HPP
