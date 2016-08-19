/* Copyright (C) 2016 Lukas Bartl
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

// Diese Datei

#include "ui_verboten.h"

/// Dieser Dialog zeigt alle verbotenen und aktuell sich im %Chat befindenden Pc-Nutzernamen an. Er wird mit der Funktion verbotene_pc_nutzernamen_dialog() erstellt.
/**
 * Es werden zuerst alle verbotenen Pc-Nutzernamen angezeigt und mit einer markierten CheckBox versehen.
 * Anschließend werden alle aktuell sich im %Chat befindenden Pc-Nutzernamen angezeigt und mit einer nicht markierten CheckBox versehen.
 * Wenn die CheckBox entmakiert wird, wird der Pc-Nutzername wieder freigegeben, wird sie markiert, wird der Pc-Nutzername aus dem %Chat entfernt und ist nun verboten.
 */
class VerbotenPc : public QDialog {
    Q_OBJECT

public:
    explicit VerbotenPc( QWidget* parent = nullptr ); ///< Konstruktor

private:
    Ui::Verboten ui {}; ///< UI des Dialogs

    ///\cond
    void schreiben();
    ///\endcond
};

#include "pc_nutzername.moc"
#include "pc_nutzername.hpp"
#include "filesystem.hpp"
#include "chat.hpp"
#include <QPushButton>

namespace {
    Datei const pc_forbidfile = "./pc_forbid";
    Datei_Mutex pc_forbidfile_mtx ( pc_forbidfile );
}

bool pc_nutzername_verboten() {
    sharable_file_mtx_lock f_lock ( pc_forbidfile_mtx );

    std::string pc_nutzername = get_pc_nutzername();
    std::ifstream is = pc_forbidfile.istream();
    std::string currname;

    while ( std::getline( is, currname ) )
        if ( currname == pc_nutzername )
            return true;

    return false;
}

/// Erstellt ein Objekt der Klasse VerbotenPc
/**
 * @param parent Parent
 */
void verbotene_pc_nutzernamen_dialog( QWidget* parent ) {
    VerbotenPc* ver = new VerbotenPc( parent );
    ver->setAttribute( Qt::WA_DeleteOnClose );
    ver->show();
}

/**
 * @param parent Parent
 */
VerbotenPc::VerbotenPc( QWidget* parent ) :
    QDialog( parent )
{
    ui.setupUi( this );
    ui.buttonBox->button( QDialogButtonBox::Save )   -> setText("Speichern");
    ui.buttonBox->button( QDialogButtonBox::Cancel ) -> setText("Abbrechen");
    this->setWindowTitle("Verbotene Pc-Nutzernamen");
    this->setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    { // Bereits verbotene Pc-Nutzernamen aus forbidfile anzeigen
        sharable_file_mtx_lock f_lock ( pc_forbidfile_mtx );
        std::ifstream is = pc_forbidfile.istream();
        std::string currname;

        while ( std::getline( is, currname ) ) {
            QListWidgetItem* curri = new QListWidgetItem( QString::fromStdString( currname ), ui.listWidget );
            curri->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
            curri->setCheckState( Qt::Checked );
        }
    }

    // Benutzer aus nutzer_verwaltung anzeigen
    shared_lock lock ( nutzer_verwaltung.read_lock() );
    std::vector <Nutzer const*> nutzer ( nutzer_verwaltung.size() ); // Alle Nutzer, die noch nicht verwendet wurden
    size_t index = 0;

    for ( Nutzer const& currnutzer : nutzer_verwaltung ) // Alle Nutzer einfügen
        nutzer[index++] = &currnutzer;

    for ( index = 0; index < nutzer.size(); ++index )
        if ( nutzer[index] ) { // nutzer[index] ist kein nullptr, also wurde noch nicht verwendet
            QString text = QString::fromStdString( nutzer[index]->pc_nutzername + " (" + nutzer[index]->nutzername );

            for ( size_t i = index+1; i < nutzer.size(); ++i ) // Alle Nutzer mit dem gleichen Pc-Nutzernamen auf nullptr setzen und deren Nutzernamen in text speichern
                if ( nutzer[i] && nutzer[i]->pc_nutzername == nutzer[index]->pc_nutzername ) {
                    text += QString::fromStdString( ", " + nutzer[i]->nutzername );
                    nutzer[i] = nullptr;
                }

            text += ")";
            QListWidgetItem* curri = new QListWidgetItem( text, ui.listWidget );
            curri->setFlags( text.contains( Chat::oberadmin ) ? Qt::ItemIsUserCheckable : Qt::ItemIsUserCheckable | Qt::ItemIsEnabled ); // Bei Oberadmin ausgegraut
            curri->setCheckState( Qt::Unchecked );
        }

    connect( this, &VerbotenPc::accepted, [this] () { schreiben(); } );
}

///\cond
void VerbotenPc::schreiben() { // speichern
    file_mtx_lock f_lock ( pc_forbidfile_mtx );
    std::ofstream os = pc_forbidfile.ostream();

    for ( int i = 0; i < ui.listWidget->count(); ++i ) {
        QListWidgetItem* curri = ui.listWidget->item( i );

        if ( curri->checkState() == Qt::Checked ) {
            std::string text = curri->text().toStdString();
            size_t pos = text.find(" (");

            if ( pos != std::string::npos ) // Nur Pc-Nutzernamen übrig lassen
                text.erase( pos );

            os << text << '\n';

            shared_lock lock ( nutzer_verwaltung.read_lock() );

            for ( Nutzer const& currnutzer : nutzer_verwaltung )
                if ( currnutzer.pc_nutzername == text ) // Nutzer informieren
                    makeToNutzerDatei( static_paths::terminatedir, currnutzer ).ostream() << 'x' << nutzer_ich.x_plum << nutzer_ich.nutzername << '\n';
        }
    }
}
///\endcond
