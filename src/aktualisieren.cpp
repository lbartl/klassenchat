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

// Diese Datei steuert das Aktualisieren des Chatverlaufs

#include "chat.hpp"
#include "chatverwaltung.hpp"
#include <QScrollBar>
#include <QDebug>

using const_it = std::string::const_iterator;

enum class Typ {
    nichts,
    info,
    von_mir,
    von_oberadmin,
    von_anders
};

namespace {
    void flushBuffer( Typ const typ, Qt::Alignment& alignvorher, const_it const block_begin, const_it const block_end, QTextBrowser*const ausgabe ) { // Buffer ausgeben
        QColor textcol;

        switch ( typ ) { // Farbe festlegen
        case Typ::info:
            textcol = Qt::darkGreen; break;
        case Typ::von_oberadmin:
            textcol = Qt::darkRed; break;
        case Typ::von_anders:
            textcol = Qt::black; break;
        case Typ::nichts:
            return; // Nichts tun
        case Typ::von_mir:
            throw std::invalid_argument("typ kann nicht Typ::von_mir sein, dafür ist flushIch() da!");
        }

        ausgabe->setTextColor( textcol ); // Farbe setzen

        Qt::Alignment const align = typ == Typ::info ? Qt::AlignHCenter : Qt::AlignLeft; // Infos zentriert, Nachrichten linksbündig

        if ( align != alignvorher ) {
            const_it const first_line_end = std::find( block_begin, block_end, '\n' ); // Ende der ersten Zeile

            ausgabe->append( QString::fromUtf8( &*block_begin, first_line_end - block_begin ) ); // Erste Zeile ausgeben
            ausgabe->setAlignment( align ); // Alignment muss nach erster Zeile erfolgen

            if ( first_line_end != block_end ) // Falls mehrere Zeilen, diese jetzt ausgeben
                ausgabe->append( QString::fromUtf8( &*first_line_end + 1, block_end - first_line_end - 1 ) );

            alignvorher = align;
        } else
            ausgabe->append( QString::fromUtf8( &*block_begin, block_end - block_begin ) ); // Alles ausgeben
    }

    void flushIch( Qt::Alignment& alignvorher, const_it const doppos, const_it const line_end, QTextBrowser*const ausgabe ) { // Eine Zeile von mir ausgeben
        QString const zeile = "Ich" + QString::fromUtf8( &*doppos, line_end - doppos ); // Aus "<benutzername>: <nachricht>" wird "Ich: <nachricht>"

        ausgabe->setTextColor( Qt::darkBlue );
        ausgabe->append( zeile ); // Zeile ausgeben

        constexpr Qt::Alignment align = Qt::AlignLeft;

        if ( align != alignvorher ) {
            ausgabe->setAlignment( align );
            alignvorher = align;
        }
    }
}

inline bool toNextLine( const_it& str_pos, const_it& line_end, const_it const str_end ) { // gibt false zurück, falls Ende des Strings
    str_pos = line_end + 1;
    line_end = std::find( str_pos, str_end, '\n' );
    return line_end != str_end;
}

/// #inhalt anzeigen.
/**
 * @param pos Position in #inhalt, bei 0 wird alles neu angezeigt, bei 1 wird auch alles neu angezeigt, die Scrollbar allerdings an die alte Stelle zurückgesetzt.
 *
 * Diese Funktion zeigt #inhalt an, indem #inhalt Zeile für Zeile durchgegangen wird:\n
 * Wenn die Zeile keinen Doppelpunkt enthält, wird sie grün ausgegeben.\n
 * Wenn die Zeile vor dem Doppelpunkt meinen Benutzernamen stehen hat, wird dieser durch "Ich" ersetzt und die Zeile blau ausgegeben.\n
 * Wenn die Zeile vor dem Doppelpunkt #oberadmin stehen hat, wird sie rot ausgegeben.\n
 * Wenn die Zeile einen anderen Benutzernamen vor dem Doppelpunkt stehen hat, wird sie schwarz ausgegeben.
 *
 * Wenn die Zeile keinen Doppelpunkt enthält, wird sie zentriert platziert, sonst linksbündig.
 */
void Chat::verlauf_up( size_t pos ) {
#ifdef DEBUG
    if ( pos > 1 )
        qDebug("Veränderungen anzeigen...");
    else
        qDebug("Chatdatei neu anzeigen...");
#endif

    if ( pos == 0 )
        ui.BrowseA->setText(""); // Bisherigen Text löschen

    int const scrollval = ui.BrowseA->verticalScrollBar()->value();
    bool const mitscroll = scrollval == ui.BrowseA->verticalScrollBar()->maximum(); // Wenn Scrollbar am Ende ist true, sonst false

    if ( pos == 1 )
        ui.BrowseA->setText(""); // Erst jetzt löschen, da es nicht mitscrollen sollte

    lock_guard inhalt_lock ( inhalt_mtx ); // Lock für inhalt

    const_it const str_end = inhalt.cend();
    const_it str_pos = pos > 1 ? inhalt.cbegin() + pos : inhalt.cbegin(),
             line_end = std::find( str_pos, str_end, '\n' ),
             block_begin; // Anfang des Buffers

    if ( str_pos == str_end ) // Wenn Chatdatei leer ist
        return;

    Qt::Alignment alignvorher = ui.BrowseA->alignment(); // wie letzte Zeile alignt ist
    Typ typvorher = Typ::nichts;

    do { // Wenn Ende von inhalt erreicht Schleife beenden
        const_it const doppos = std::find( str_pos, line_end, ':' ); // Position des Doppelpunkts
        Typ currtyp;

        if ( doppos == line_end ) // Wenn kein Doppelpunkt vorhanden ist, ist es eine Info
            currtyp = Typ::info;
        else if ( std::equal( str_pos, doppos, nutzer_ich.nutzername.begin(), nutzer_ich.nutzername.end() ) ) // Nachricht stammt von mir selber
            currtyp = Typ::von_mir;
        else if ( ! nutzer_ich.x_plum && chat_verwaltung.imKlassenchat() && std::equal( str_pos, doppos, std::begin( oberadmin ), std::end( oberadmin ) - 1 ) ) // Nachricht stammt vom Oberadmin
            currtyp = Typ::von_oberadmin;
        else // Nachricht stammt von jemand anders
            currtyp = Typ::von_anders;

        if ( currtyp != typvorher ) { // Neuer Typ, Buffer ausgeben
            flushBuffer( typvorher, alignvorher, block_begin, str_pos - 1, ui.BrowseA );
            typvorher = currtyp;
            block_begin = str_pos;
        }

        if ( currtyp == Typ::von_mir ) { // Zeile stammt von mir, muss sofort ausgegeben werden
            flushIch( alignvorher, doppos, line_end, ui.BrowseA );
            typvorher = Typ::nichts;
        }
    } while ( toNextLine( str_pos, line_end, str_end ) );

    flushBuffer( typvorher, alignvorher, block_begin, str_end - 1, ui.BrowseA ); // Rest aus dem Buffer ausgeben

    ui.BrowseA->verticalScrollBar()->setValue( mitscroll ? ui.BrowseA->verticalScrollBar()->maximum() : scrollval ); // Ans Ende scrollen bzw. zurück an den Punkt scrollen, wo man vorher war
}
