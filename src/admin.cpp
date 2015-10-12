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

// Diese Datei definiert die Klasse Admin

#include "admin.hpp"
#include "chat.hpp"
#include "adminpass.hpp"
#include "filesystem.hpp"
#include "klog.hpp"
#include <QPushButton>

/**
 * @param nutzer Chat::nutzer_h
 * @param admins Chat::admins_h
 * @param admin_pass Chat::passwords
 * @param parent Parent
 */
Admin::Admin( Hineinschreiben const& nutzer, Hineinschreiben const& admins, AdminPass const& admin_pass, QWidget* parent ) :
    QDialog( parent ),
    admins_v( admins.get_namen() )
{
    ui.setupUi( this );
    ui.buttonBox -> button( QDialogButtonBox::Cancel ) -> setText("Abbrechen");

    this -> setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    for ( QString const& currnutzer : nutzer ) {
        std::string const name = fromBenutzername_str( currnutzer ).second;
        QString const& anz_name { enthaelt( Chat::std_admins.begin(), Chat::std_admins.end(), name ) ? currnutzer + QString::fromStdString( " (Passwort \"" + admin_pass.getpass( name ) + "\")" )
                                                                                                     : currnutzer }; // Bei einem std_admin das Passwort anhängen, sonst nur den Benutzernamen
        QListWidgetItem* curri = new QListWidgetItem( anz_name, ui.listWidget );

        curri -> setFlags( name == Chat::oberadmin ? Qt::ItemIsUserCheckable : Qt::ItemIsUserCheckable | Qt::ItemIsEnabled ); // Sich selbst kann der Oberadmin nicht entmachten
        curri -> setCheckState( admins.hineingeschrieben( currnutzer ) ? Qt::Checked : Qt::Unchecked ); // je nachdem, ob currnutzer ein Admin ist oder nicht, das Item checken
    }

    ui.listWidget -> sortItems();

    connect( this, &Admin::accepted, [this] () { schreiben(); } );
}

///\cond
void Admin::schreiben() const { // Information an Nutzer schreiben, dass sein Admin-Status sich ändert
    size_t const count = ui.listWidget -> count();

    for ( size_t i = 0; i < count; ++i ) {
        QListWidgetItem const*const curri = ui.listWidget -> item( i );
        QString currnutzer = curri -> text();

        int const an_pos = currnutzer.indexOf("(Pa");

        if ( an_pos != -1 ) // Bei std_admins das Passwort-Anhängsel wieder löschen
            currnutzer.remove( an_pos - 1, currnutzer.length() );

        bool const is_admin = enthaelt( admins_v.begin(), admins_v.end(), currnutzer ); // Ob currnutzer momentan ein Admin ist
        bool const marked = curri -> checkState() == Qt::Checked; // Ob er gecheckt ist
        bool to_admin; // Was geschehen soll

        if ( ! is_admin && marked ) // Zum Admin machen
            to_admin = true;
        else if ( is_admin && ! marked ) // Zum normalen Nutzer machen
            to_admin = false;
        else // Nichts tun
            continue;

#ifdef _DEBUG // Gibt Debug-Informationen aus und kann deswegen nicht currnutzer als rvalue nutzen
        makeToNutzerDatei( static_paths::infodir, fromBenutzername_str( currnutzer ) ).ostream() << to_admin;
        KLOG << currnutzer.toStdString() << ( to_admin ? " zum Admin gemacht!" : " zum normalen Nutzer gemacht!" ) << endl;
#else
        makeToNutzerDatei( static_paths::infodir, fromBenutzername_str( std::move( currnutzer ) ) ).ostream() << to_admin;
#endif
    }
}
///\endcond
