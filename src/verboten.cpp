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

// Diese Datei definiert die Klasse Verboten

#include "verboten.hpp"
#include "filesystem.hpp"
#include "klog.hpp"
#include <QPushButton>
#include <regex>

QString const Verboten::neu_text {"Neuer Eintrag..."};
///\cond
decltype( Verboten::std_admins ) Verboten::std_admins {};
///\endcond

using static_paths::verbotenfile;

Verboten::Verboten( QWidget* parent ) :
    QDialog( parent )
{
    ui.setupUi( this );
    ui.buttonBox -> button( QDialogButtonBox::Save )   -> setText("Speichern");
    ui.buttonBox -> button( QDialogButtonBox::Cancel ) -> setText("Abbrechen");

    if ( std_admins[0] == "" ) // noch nicht initialisiert
        std::copy( Chat::std_admins.begin(), Chat::std_admins.end(), std_admins.begin() );

    this -> setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    for ( QString const& currname : Hineinschreiben( verbotenfile ) ) {
        QListWidgetItem* curri = new QListWidgetItem( currname, ui.listWidget );
        curri -> setFlags( currname == "Ich" ? Qt::ItemIsEditable : Qt::ItemIsEnabled | Qt::ItemIsEditable ); // "Ich" ist immer verboten
        ++count;
    }

    ui.listWidget -> sortItems();
    create_itemneu();

    connect( ui.listWidget, &QListWidget::itemChanged, this, &Verboten::aktualisieren );
    connect( this, &Verboten::accepted, [this] () { schreiben(); } );
}

void Verboten::create_itemneu() { // Das itemneu erstellen
    itemneu = new QListWidgetItem( neu_text, ui.listWidget );

    QFont schrift;
    schrift.setItalic( true );

    itemneu -> setFont( schrift );
    itemneu -> setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable );
}

void Verboten::aktualisieren( QListWidgetItem*const item ) {
    if ( item != itemneu ) {
        QString const text = item -> text(), // Orginal-Text
                      itemtext = text.trimmed(); // Text des Items (ohne Leerzeichen am Anfang und Ende)

        { // Falls ungültig das Item löschen
            bool fail = itemtext.isEmpty() || // Keinen Text eingegeben
                        std::any_of( std_admins.begin(), std_admins.end(), caseInsEqualFunc{ itemtext } ) || // Einen std_admin eingegeben
                        std::regex_match( itemtext.toStdString(), std::regex("[\\wÄäÖöÜüß_]+") ); // Ungültigen Text eingegeben

            if ( ! fail )
                for ( unsigned int i = 0; i < count; ++i ) {
                    QListWidgetItem* curri = ui.listWidget -> item( i );
                    if ( curri != item && caseInsEqual( curri -> text(), itemtext ) ) // Item mit diesem Text existiert schon
                        fail = true;
                }

            if ( fail ) { // löschen
                klog("Item ist ungültig und wird deswegen wieder gelöscht!");
                delete item;
                --count;
                return;
            }
        }

        if ( itemtext != text )
            item -> setText( itemtext ); // Leerzeichen vorne und hinten entfernen

        ui.listWidget -> takeItem( neupos ); // itemneu aus der Liste nehmen, damit es nicht mitsortiert wird
        ui.listWidget -> sortItems(); // restliche Items sortieren
        ui.listWidget -> insertItem( neupos, itemneu ); // itemneu wieder ans Ende anfügen
    } else if ( item -> text() != neu_text ) { // item==itemneu und Text wurde verändert
        create_itemneu(); // Ein neues itemneu erstellen
        ++count;
        item -> setFont( QFont() ); // kursiv wegmachen und indirekt aktualisieren() aufrufen, was dann entweder das Item löscht oder alle Items neu sortiert
    }
}

///\cond
void Verboten::schreiben() { // speichern
    std::ofstream verbotendatei = verbotenfile.ostream();
    Datei_Mutex verbotenfile_mtx ( verbotenfile );
    file_mtx_lock f_lock ( verbotenfile_mtx );

    for ( unsigned int i = 0; i < count; ++i )
        verbotendatei << ui.listWidget -> item( i ) -> text().toStdString() << '\n';

    verbotendatei.flush();

    klog("verbotenfile aktualisiert!");
}
///\endcond
