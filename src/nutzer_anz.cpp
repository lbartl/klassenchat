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

// Diese Datei definiert die Klassen Admin_anz und Nutzer_anz

#include "nutzer_anz.hpp"
#include "chat.hpp"
#include "adminpass.hpp"
#include "filesystem.hpp"
#include "global.hpp"
#include "klog.hpp"
#include <QPushButton>

/**
 * @param parent Parent
 */
Nutzer_anz::Nutzer_anz( QWidget* parent ) :
    QDialog( parent )
{
    ui.setupUi( this );

    this->setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    shared_lock lock ( nutzer_verwaltung.read_lock() );

    for ( Nutzer const& currnutzer : nutzer_verwaltung ) {
        std::string name = currnutzer.nutzername + " (" + currnutzer.pc_nutzername + ')';

        if ( currnutzer.x_plum )
            name += " (Plum-Chat)"; // Falls currnutzer im Plum-Chat ist, " (Plum-Chat)" anhängen

        if ( currnutzer.admin )
            name += " (Admin)"; // Falls currnutzer ein Admin ist, " (Admin)" anhängen

        ui.listWidget->addItem( QString::fromStdString( name ) );
    }

    ui.listWidget->sortItems();
}

/**
 * @param admin_pass Chat::passwords
 * @param parent Parent
 */
Admin_anz::Admin_anz( AdminPass const& admin_pass, QWidget* parent ) :
    QDialog( parent )
{
    ui.setupUi( this );
    ui.buttonBox->button( QDialogButtonBox::Cancel )->setText("Abbrechen");

    this->setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    shared_lock lock ( nutzer_verwaltung.read_lock() );

    anz_nutzer.reserve( nutzer_verwaltung.size() ); // Genügend Speicherplatz reservieren

    for ( Nutzer const& currnutzer : nutzer_verwaltung ) {
        std::string name = currnutzer.nutzername + " (" + currnutzer.pc_nutzername + ')';

        if ( currnutzer.x_plum )
            name += " (Plum-Chat)";

        if ( ! currnutzer.x_plum && enthaelt( Chat::std_admins, currnutzer.nutzername ) )
            name += " (Passwort \"" + admin_pass.getpass( currnutzer.nutzername ) + "\")";

        anz_nutzer.emplace_back( currnutzer.nummer, QListWidgetItem( QString::fromStdString( name ) ) ); // Item erstellen und mit Nummer des Nutzers in anz_nutzer schreiben
        QListWidgetItem& curri = anz_nutzer.back().second; // Referenz auf das Item
        ui.listWidget->addItem( &curri ); // Item hinzufügen
        curri.setFlags( &currnutzer == &nutzer_ich ? Qt::ItemIsUserCheckable : Qt::ItemIsUserCheckable | Qt::ItemIsEnabled ); // Sich selbst kann der Oberadmin nicht entmachten
        curri.setCheckState( currnutzer.admin ? Qt::Checked : Qt::Unchecked ); // je nachdem, ob currnutzer ein Admin ist oder nicht, das Item checken
    }

    ui.listWidget->sortItems();

    connect( this, &Admin_anz::accepted, [this] () { schreiben(); } );
}

///\cond
void Admin_anz::schreiben() const { // Information an Nutzer schreiben, dass sein Admin-Status sich ändert
    shared_lock lock ( nutzer_verwaltung.read_lock() );

    for ( auto const& pair : anz_nutzer ) {
        ui.listWidget->takeItem( 0 ); // Entfernen, damit ui.listWidget nicht versucht es zu löschen

        Nutzer const* currnutzer = nutzer_verwaltung.getNutzer( pair.first );

        if ( ! currnutzer ) // Nutzer hat den Chat verlassen
            continue;

        bool const marked = pair.second.checkState() == Qt::Checked; // Ob er gecheckt ist
        bool to_admin; // Was geschehen soll

        if ( ! currnutzer->admin && marked ) // Zum Admin machen
            to_admin = true;
        else if ( currnutzer->admin && ! marked ) // Zum normalen Nutzer machen
            to_admin = false;
        else // Nichts tun
            continue;

        makeToNutzerDatei( static_paths::infodir, *currnutzer ).ostream() << to_admin;
        KLOG << ( currnutzer->x_plum ? currnutzer->nutzername + " (Plum-Chat)" : currnutzer->nutzername ) << ( to_admin ? " zum Admin gemacht!" : " zum normalen Nutzer gemacht!" ) << endl;
    }
}
///\endcond
