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

///\file
// Dieser Header deklariert die Klasse SimpleDialog und die Funktion createDialog

#ifndef SIMPLEDIALOG_HPP
#define SIMPLEDIALOG_HPP

#include "ui_simpledialog.h"

/// Mit der Klasse SimpleDialog können einfache Dialoge erstellt werden.
/**
 * SimpleDialog wird für einfache Dialoge, wie eine Bestätigung oder eine Information, verwendet.
 *
 * Der Titel des Dialogs und der Text, der in einem QLabel steht, werden dem Konstruktor zusammen mit dem Parent übergeben.
 * Außerdem hat der Dialog einen Ok-Button.
 */
class SimpleDialog : public QDialog
{
    Q_OBJECT

public:
    /// Allgemeiner Konstruktor.
    explicit SimpleDialog( QString const& titel, QString const& text, QWidget* parent = nullptr );

private:
    Ui::SimpleDialog ui {}; ///< UI des Dialogs
};

/// Einen SimpleDialog erstellen.
/**
 * @param titel Titel für SimpleDialog
 * @param text Text für SimpleDialog
 * @param parent Parent für SimpleDialog
 * @param modal Wenn true, ist der erzeugte Dialog modal, sonst nicht
 *
 * Erstellt einen SimpleDialog auf dem Heap (mit Qt::WA_DeleteOnClose) und gibt titel, text und parent an den Konstruktor weiter.
 * Danach wird, wenn modal true ist, Qt::ApplicationModal gesetzt.
 * Als letztes wird der Dialog angezeigt.
 */
void createDialog( QString const& titel, QString const& text, QWidget* parent, bool modal = false );

#endif // SIMPLEDIALOG_HPP
