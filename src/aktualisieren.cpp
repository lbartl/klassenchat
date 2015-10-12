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

// Diese Datei steuert das Aktualisieren des Chatverlaufs

#include "chat.hpp"
#include "filesystem.hpp"
#include "klog.hpp"
#include <QScrollBar>
#include <QTimer>

using const_it = std::string::const_iterator;

enum class Typ : uint_fast8_t {
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

        ausgabe -> setTextColor( textcol ); // Farbe setzen

        Qt::Alignment const align = typ == Typ::info ? Qt::AlignHCenter : Qt::AlignLeft; // Infos zentriert, Nachrichten linksbündig

        if ( align != alignvorher ) {
            const_it const first_line_end = std::find( block_begin, block_end, '\n' ); // Ende der ersten Zeile

            ausgabe -> append( QString::fromUtf8( &*block_begin, first_line_end - block_begin ) ); // Erste Zeile ausgeben
            ausgabe -> setAlignment( align ); // Alignment muss nach erster Zeile erfolgen

            if ( first_line_end != block_end ) // Falls mehrere Zeilen, diese jetzt ausgeben
                ausgabe -> append( QString::fromUtf8( &*first_line_end + 1, block_end - first_line_end - 1 ) );

            alignvorher = align;
        } else
            ausgabe -> append( QString::fromUtf8( &*block_begin, block_end - block_begin ) ); // Alles ausgeben
    }

    void flushIch( Qt::Alignment& alignvorher, const_it const doppos, const_it const line_end, QTextBrowser*const ausgabe ) { // Eine Zeile von mir ausgeben
        QString const zeile = "Ich" + QString::fromUtf8( &*doppos, line_end - doppos ); // Aus "<benutzername>: <nachricht>" wird "Ich: <nachricht>"

        ausgabe -> setTextColor( Qt::darkBlue );
        ausgabe -> append( zeile ); // Zeile ausgeben

        constexpr Qt::Alignment align = Qt::AlignLeft;

        if ( align != alignvorher ) {
            ausgabe -> setAlignment( align );
            alignvorher = align;
        }
    }
}

inline bool toNextLine( const_it& str_pos, const_it& line_end, const_it const str_end ) { // gibt false zurück, falls Ende des Strings
    str_pos = line_end + 1;
    line_end = std::find( str_pos, str_end, '\n' );
    return line_end != str_end;
}

/// Chatverlauf aktualisieren.
/**
 * verlauf_up() ist die Hauptfunktion der Klasse Chat.
 * Sie aktualisiert den Chatverlauf, indem sie #chatfile einliest, und ruft dabei alle 0,1 Sekunden pruefen_main() auf.
 *
 * Wenn pruefen_main() true zurückgibt, wird stop() aufgerufen.
 *
 * Wenn #x_reload gesetzt ist, wird der ganze Chatverlauf neu geladen.
 */
void Chat::verlauf_up() {
    static std::string inhalt_old; // In dieser Variable ist immer das gespeichert, was vor 0,1 Sekunden in der Chatdatei war
    static uint_fast16_t index = 0; // Alle 100 Sekunden (jedes 1000. Mal) wird der gesamte Chatverlauf aktualisiert (x_neu), da manchmal Fehler auftreten

    bool const x_neu = ++index == 1000;
    bool const neu = x_neu || flags[x_reload]; // x_reload für auch nach unten scrollen, x_neu nicht

    if ( neu ) {
        flags.reset( x_reload );
        index = 0;
    }

    std::string inhalt_new = chatfile -> readAll(); // ohne Datei_Mutex, da sowieso nach 0,1 Sekunden erneut eingelesen und gegebenenfalls aktualisiert wird.

    if ( neu || inhalt_new != inhalt_old ) { // prüfen, ob sich Datei in 0,1 Sekunden geändert hat
        const_it str_pos;

        if ( ! neu && std::equal( inhalt_old.begin(), inhalt_old.end(), inhalt_new.begin() ) ) { // Der Rest des Chatverlaufs ist gleichgeblieben
            klog("Veränderungen anzeigen...");
            str_pos = inhalt_new.begin() + inhalt_old.length(); // Ab Veränderung
        } else {
            klog("Chatdatei neu anzeigen...");
            str_pos = inhalt_new.begin(); // Von vorne
            if ( ! x_neu ) ui.BrowseA -> setText(""); // Bisherigen Text löschen
        }

        int const scrollval = ui.BrowseA -> verticalScrollBar() -> value();
        bool const mitscroll = scrollval == ui.BrowseA -> verticalScrollBar() -> maximum(); // Wenn Scrollbar am Ende ist true, sonst false

        if ( x_neu ) ui.BrowseA -> setText(""); // Erst jetzt löschen, da es nicht mitscrollen sollte

        Qt::Alignment alignvorher = ui.BrowseA -> alignment(); // wie letzte Zeile alignt ist

        const_it const str_end = inhalt_new.cend();
        const_it line_end = std::find( str_pos, str_end, '\n' );

        const_it block_begin; // Anfang des Buffers
        Typ typvorher = Typ::nichts;

        do { // Wenn Ende von inhalt_new erreicht Schleife beenden
            const_it const doppos = std::find( str_pos, line_end, ':' ); // Position des Doppelpunkts
            Typ currtyp;

            if ( doppos == line_end ) // Wenn kein Doppelpunkt vorhanden ist, ist es eine Info
                currtyp = Typ::info;
            else if ( std::equal( str_pos, doppos, nutzername.begin(), nutzername.end() ) ) // Nachricht stammt von mir selber
                currtyp = Typ::von_mir;
            else if ( ! x_plum && flags[chatall] && std::equal( str_pos, doppos, std::begin( oberadmin ), std::end( oberadmin ) - 1 ) ) // Nachricht stammt vom Oberadmin
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

        ui.BrowseA -> verticalScrollBar() -> setValue( mitscroll ? ui.BrowseA -> verticalScrollBar() -> maximum() : scrollval ); // Ans Ende scrollen bzw. zurück an den Punkt scrollen, wo man vorher war

        inhalt_old.swap( inhalt_new ); // inhalt_new wird nicht mehr benötigt, inhalt_old den Wert zuweisen
    }
}

/// Undocumented.
void Chat::main_thread() {
    if ( pruefen_main() )
        QTimer::singleShot( 0, [this] () { stop(); } ); // start.cpp
    else {
        verlauf_up();
        QTimer::singleShot( 100, [this] () { main_thread(); } ); // sich selbst in 0,1 Sekunden wieder aufrufen
    }
}
