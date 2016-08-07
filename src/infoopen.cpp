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

// Diese Datei steuert den QDialog um eine Information an einen Nutzer zu senden

#include "infoopen.hpp"
#include "filesystem.hpp"
#include "klog.hpp"
#include <QPushButton>

/**
 * @param an Voreingestellter Benutzername
 * @param parent Parent
 */
InfoOpen::InfoOpen( std::string const& an, QWidget* parent ) :
    QDialog( parent )
{
    ui.setupUi( this );
    ui.buttonBox -> button( QDialogButtonBox::Cancel ) -> setText("Abbrechen");

    this -> setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    ui.comboBox -> insertItem( 0, "" );

    shared_lock lock ( nutzer_verwaltung.read_lock() );

    for ( Nutzer const& currnutzer : nutzer_verwaltung )
        if ( currnutzer.x_plum != nutzer_ich.x_plum )
            continue;
        else if ( currnutzer.nutzername == an )
            ui.comboBox -> setItemText( 0, QString::fromStdString( an ) );
        else if ( &currnutzer != &nutzer_ich )
            ui.comboBox -> addItem( QString::fromStdString( currnutzer.nutzername ) );

    ui.comboBox -> setCurrentIndex( 0 );
    ui.comboBox -> model() -> sort( 0 );

    ui.plainTextEdit -> setLineWrapMode( QPlainTextEdit::NoWrap ); // Horizontale Scrollbar statt neuer Zeile
    ui.plainTextEdit -> setFocus();

    connect( this, &InfoOpen::accepted, [this] () { schreiben(); } );
}

///\cond
void InfoOpen::schreiben() const { // Information an Nutzer schreiben
    Nutzer const*const nutzer = nutzer_verwaltung.getNutzer( nutzer_ich.x_plum, ui.comboBox->currentText().toStdString() ); // Ausgewählter Nutzer

    if ( ! nutzer ) {
        qWarning("Nutzer ist nicht mehr im Chat!");
        return;
    }

    std::string const text = ui.plainTextEdit->toPlainText().toStdString(); // Eingegebener Text

    if ( text.empty() ) {
        qWarning("Keinen Text eingegeben!");
        return;
    }

    makeToNutzerDatei( static_paths::infodir, *nutzer ).ostream() << 'i' << nutzer_ich.nutzername << '\n' << text;

    klog("Information gesendet!");
}
///\endcond
