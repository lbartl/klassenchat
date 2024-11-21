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

// Diese Datei definiert die Klasse Verboten

#include "ui_verboten.h"
#include "chat.hpp"

/// Dieser Dialog zeigt alle verbotenen Benutzernamen an. Er wird mit der Funktion verbotene_namen_dialog() erstellt.
/**
 * Es können Benutzernamen hinzugefügt werden, indem das #itemneu bearbeitet wird.
 * Es können Benutzernamen entfernt werden, indem der Text gelöscht wird.
 * Es können Benutzernamen verändert werden, indem der Text bearbeitet wird.
 */
class Verboten : public QDialog {
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

    static inline QString const neu_text {"Neuer Eintrag..."}; ///< Text des #itemneu
    static inline std::array <QString, Chat::std_admins.size()> std_admins {}; ///< Chat::std_admins mit QString

    void create_itemneu(); ///< #itemneu erstellen

    ///\cond
    void schreiben();
    ///\endcond

private slots:
    void aktualisieren( QListWidgetItem* item ); ///< Wird aufgerufen, wenn ein Item verändert wurde
};

#include "verboten.moc"
#include "global.hpp"
#include <QPushButton>
#include <QDebug>

/// Erstellt ein Objekt der Klasse Verboten.
/**
 * @param parent Parent
 */
void verbotene_namen_dialog( QWidget* parent ) {
    Verboten* ver = new Verboten( parent );
    ver->setAttribute( Qt::WA_DeleteOnClose );
    ver->show();
}

namespace {
    Datei const verbotenfile = "./forbid";
    Datei_Mutex verbotenfile_mtx ( verbotenfile );
}

/// Überprüft ob ein Name verboten ist.
/**
 * @param name Der zu überprüfende Name
 * @return Ob der Name verboten ist
 */
bool verboten( std::string const& name ) {
    sharable_file_mtx_lock f_lock ( verbotenfile_mtx );

    if ( ! verbotenfile.exist() )
        verbotenfile.write("Ich"); // Ich ist immer verboten

    std::ifstream is = verbotenfile.istream();
    std::string currname;

    while ( is >> currname ) // Alle Namen einlesen
        if ( currname.size() == name.size() ) // Nur bei gleicher Größe vergleichen
            if ( caseInsEqual( QString::fromStdString( currname ), QString::fromStdString( name ) ) )
                return true;

    return false;
}

Verboten::Verboten( QWidget* parent ) :
    QDialog( parent )
{
    ui.setupUi( this );
    ui.buttonBox -> button( QDialogButtonBox::Save )   -> setText("Speichern");
    ui.buttonBox -> button( QDialogButtonBox::Cancel ) -> setText("Abbrechen");

    if ( std_admins[0].isEmpty() ) // noch nicht initialisiert
        for (size_t i = 0; i < std_admins.size(); ++i)
            std_admins[i] = Chat::std_admins[i].data();

    this -> setWindowTitle("Verbotene Nutzernamen");
    this -> setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    sharable_file_mtx_lock f_lock ( verbotenfile_mtx );
    std::ifstream is = verbotenfile.istream();
    std::string currname;

    while ( is >> currname ) {
        QListWidgetItem* curri = new QListWidgetItem( QString::fromStdString( currname ), ui.listWidget );
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
                        itemtext.length() > 20 || // Zu langer Benutzername
                        std::any_of( std_admins.begin(), std_admins.end(), caseInsEqualFunc{ itemtext } ) || // Einen std_admin eingegeben
                        ! regex_nutzername.exactMatch( itemtext ); // Ungültigen Text eingegeben

            if ( ! fail )
                for ( unsigned int i = 0; i < count; ++i ) {
                    QListWidgetItem* curri = ui.listWidget -> item( i );
                    if ( curri != item && caseInsEqual( curri -> text(), itemtext ) ) // Item mit diesem Text existiert schon
                        fail = true;
                }

            if ( fail ) { // löschen
                qDebug("Item ist ungültig und wird deswegen wieder gelöscht!");
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
    file_mtx_lock f_lock ( verbotenfile_mtx );
    std::ofstream verbotendatei = verbotenfile.ostream();

    for ( unsigned int i = 0; i < count; ++i )
        verbotendatei << ui.listWidget -> item( i ) -> text().toStdString() << '\n';

    qDebug("verbotenfile aktualisiert!");
}
///\endcond
