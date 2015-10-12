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

// Diese Datei steuert den QDialog um eine Information an einen Nutzer zu senden

#include "infoopen.hpp"
#include "hineinschreiben.hpp"
#include "filesystem.hpp"
#include "klog.hpp"
#include <QPushButton>

/**
 * @param benutzername_str Chat::nutzername_str
 * @param nutzer Chat::nutzer_h
 * @param an Voreingestellter Benutzername
 * @param parent Parent
 */
InfoOpen::InfoOpen( QString const& benutzername_str, Hineinschreiben const& nutzer, std::string const& an, QWidget* parent ) :
    QDialog( parent ),
    nutzername_str( benutzername_str )
{
    ui.setupUi( this );
    ui.buttonBox -> button( QDialogButtonBox::Cancel ) -> setText("Abbrechen");

    this -> setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    QString const name_an = QString::fromStdString( an );

    ui.comboBox -> insertItem( 0, "" );

    for ( QString const& currnutzer : nutzer )
        if ( currnutzer == name_an )
            ui.comboBox -> setItemText( 0, name_an );
        else if ( currnutzer != nutzername_str )
            ui.comboBox -> addItem( currnutzer );

    ui.comboBox -> setCurrentIndex( 0 );
    ui.comboBox -> model() -> sort( 0 );

    ui.plainTextEdit -> setLineWrapMode( QPlainTextEdit::NoWrap ); // Horizontale Scrollbar statt neuer Zeile
    ui.plainTextEdit -> setFocus();

    connect( this, &InfoOpen::accepted, [this] () { schreiben(); } );
}

///\cond
void InfoOpen::schreiben() const { // Information an Nutzer schreiben
    QString name_to = ui.comboBox -> currentText(); // Ausgewählter Benutzername

    if ( name_to == "" ) {
        qWarning("Keinen Namen ausgewählt!");
        return;
    }

    std::string const text = ui.plainTextEdit -> toPlainText().toStdString(); // Eingegebener Text

    if ( text == "" ) {
        qWarning("Keinen Text eingegeben!");
        return;
    }

    makeToNutzerDatei( static_paths::infodir, fromBenutzername_str( std::move( name_to ) ) ).ostream() << 'i' << nutzername_str.toStdString() << '\n' << text;

    klog("Information gesendet!");
}
///\endcond
