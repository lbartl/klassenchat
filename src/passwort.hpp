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

// Dieser Header deklariert die Klasse Passwort

#ifndef PASSWORT_HPP
#define PASSWORT_HPP

#include "ui_passwort.h"

/// Mit der Klasse Passwort kann ein Passwort-Dialog erzeugt werden.
/**
 * Der Passwort-Dialog besteht aus einem QLabel, der Standard-Text ist "Passwort: " und kann mit setText() geändert werden.
 * Darunter ist eine QLineEdit mit EchoMode::Password.
 * Der Standard-Titel ist "Handout Kräuterhexe".
 */
class Passwort : public QDialog
{
    Q_OBJECT

public:
    explicit Passwort( QWidget* parent = nullptr ); ///< Konstruktor

    /// Eingegebenes %Passwort zurückgeben.
    std::string const& getpass() const {
        return passein;
    }

    /// Text für QLabel setzen
    void setText( QString const& text ) {
        ui.labelText -> setText( text );
    }

private:
    Ui::Passwort ui {}; ///< UI des Dialogs
    std::string passein {}; ///< Eingegbenes %Passwort

    void setzen(); ///< Setzt #passein, aufgerufen wenn der Ok-Button angeklickt wurde

signals:
    void eingegeben( std::string const& newpass ); ///< Ein Signal mit dem eingegebenem %Passwort, von setzen() gesendet
};

#endif // PASSWORT_HPP
