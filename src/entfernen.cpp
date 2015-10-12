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

// Diese Datei definiert die Klasse Entfernen

#include "entfernen.hpp"
#include "hineinschreiben.hpp"
#include "filesystem.hpp"
#include "klog.hpp"
#include <QPushButton>

/**
 * @param benutzername_str Chat::nutzername_str
 * @param nutzer Chat::nutzer_h
 * @param ter_name Voreingestellter Benutzername
 * @param parent Parent
 */
Entfernen::Entfernen( QString const& benutzername_str, Hineinschreiben const& nutzer, std::string const& ter_name, QWidget* parent ) :
    QDialog( parent ),
    nutzername_str( benutzername_str )
{
    ui.setupUi( this );
    ui.buttonBox -> button( QDialogButtonBox::Cancel ) -> setText("Abbrechen");

    this -> setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    QString const name_t = QString::fromStdString( ter_name );

    ui.comboBox -> insertItem( 0, "" );

    for ( QString const& currnutzer : nutzer )
        if ( currnutzer == name_t )
            ui.comboBox -> setItemText( 0, name_t );
        else if ( currnutzer != nutzername_str )
            ui.comboBox -> addItem( currnutzer );

    ui.comboBox -> setCurrentIndex( 0 );
    ui.comboBox -> model() -> sort( 0 ); // Alphabetisch sortieren

    connect( this, &Entfernen::accepted, [this] () { schreiben(); } );
}

///\cond
void Entfernen::schreiben() const { // Den Nutzer entfernen
    QString name_t = ui.comboBox -> currentText();

    if ( name_t == "" ) {
        qWarning("Keinen Namen ausgewählt!");
        return;
    }

    makeToNutzerDatei( static_paths::terminatedir, fromBenutzername_str( std::move( name_t ) ) ).write( nutzername_str.toStdString() ); // Schreibe hinein, wer ihn entfernt hat

    klog("Terminate-Datei erstellt!");
}
///\endcond
