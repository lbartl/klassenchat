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

// Diese Datei definiert die Klasse Entfernen

#include "entfernen.hpp"
#include "nutzer.hpp"
#include "filesystem.hpp"
#include "klog.hpp"
#include <QPushButton>

/**
 * @param ter_name Voreingestellter Benutzername
 * @param parent Parent
 */
Entfernen::Entfernen( std::string const& ter_name, QWidget* parent ) :
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
        else if ( currnutzer.nutzername == ter_name )
            ui.comboBox -> setItemText( 0, QString::fromStdString( currnutzer.nutzername ) );
        else if ( &currnutzer != &nutzer_ich )
            ui.comboBox -> addItem( QString::fromStdString( currnutzer.nutzername ) );

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

    makeToNutzerDatei( static_paths::terminatedir, nutzer_ich.x_plum, name_t.toStdString() ).write( nutzer_ich.nutzername ); // Schreibe hinein, wer ihn entfernt hat

    klog("Terminate-Datei erstellt!");
}
///\endcond
