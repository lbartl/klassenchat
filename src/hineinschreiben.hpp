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

///\file
// Dieser Header deklariert die Klasse Hineinschreiben

#ifndef HINEINSCHREIBEN_HPP
#define HINEINSCHREIBEN_HPP

#include "datei_mutex.hpp"
#include "global.hpp"

/// In Benutzername_str umwandeln
/**
 * @param plum ob der %Nutzer im Plum-Chat ist
 * @param benutzername der Benutzername
 * @returns den Benutzername_str.
 *
 * Der Benutzername_str ist ein QString mit dem Benutzernamem, und bei Leuten aus dem Plum-Chat mit einem " (Plum-Chat)" angehängt.
 */
inline QString toBenutzername_str( bool const plum, std::string const& benutzername ) {
    QString benutzername_str = QString::fromStdString( benutzername );
    if ( plum ) benutzername_str += " (Plum-Chat)";
    return benutzername_str;
}

/// Benutzername_str zurück umwandeln
/**
 * @param benutzername_str der Benutzername_str
 * @returns ein std::pair mit einem bool, der anzeigt ob der %Nutzer im Plum-Chat ist,
 * und einem std::string, der den Benutzernamen enthält.
 */
inline std::pair <bool, std::string> fromBenutzername_str( QString const& benutzername_str ) {
    int const pos = benutzername_str.indexOf(' ');
    bool const plum = pos != -1;
    return std::pair <bool, std::string> ( plum, plum ? benutzername_str.left( pos ).toStdString() : benutzername_str.toStdString() );
}

/// Mit der Klasse Hineinschreiben können Dateien verwaltet werden, in der Benutzernamen stehen.
/**
 * Hineinschreiben definiert Methoden für solche Dateien, wie das hineinschreiben des eigenen Benutzernamen
 * und das überprüfen, ob ein anderer Benutzername in der %Datei steht.
 *
 * Es werden alle Benutzernamen in den Arbeitsspeicher geladen, zum Aktualisieren siehe \ref aktualisieren.
 */
class Hineinschreiben
{
public:
    explicit Hineinschreiben( QString const& benutzername_str, Datei const& datei ); ///< Allgemeiner Konstruktor
    explicit Hineinschreiben( Datei const& datei ); ///< Konstruktor nur mit einer %Datei
    Hineinschreiben( Hineinschreiben&& other ) = default; ///< Move-Konstruktor

    /// Überprüfen ob ein Benutzername_str in der %Datei steht.
    /**
     * @param benutzername_str der Benutzername_str
     * @param caseIns wenn true wird \ref caseInsEqual benutzt, sonst werden die Benutzernamen genau verglichen
     * @returns true, wenn Benutzername_str in %Datei vorhanden ist, sonst false.
     */
    bool hineingeschrieben( QString const& benutzername_str, bool caseIns = false ) const {
        return caseIns ? std::any_of( namen.begin(), namen.end(), caseInsEqualFunc{ benutzername_str } )
                       : enthaelt( namen, benutzername_str );
    }

    /// Überprüfen ob mein Benutzername in der %Datei steht.
    /**
     * @returns true, wenn mein Benutzername in %Datei vorhanden ist, sonst false.
     */
    bool hineingeschrieben() const {
        return hineingeschrieben( nutzername_str );
    }

    /// Überprüfen ob ein Benutzername in der %Datei steht.
    /**
     * @param plum ob der %Nutzer im Plum-Chat ist
     * @param benutzername der Benutzername
     * @returns true, wenn Benutzername in %Datei vorhanden ist, sonst false.
     */
    bool hineingeschrieben( bool plum, std::string const& benutzername ) const {
        return hineingeschrieben( toBenutzername_str( plum, benutzername ) );
    }

    /// konstanter Iterator zum ersten Benutzername_str.
    auto begin() const {
        return namen.cbegin();
    }

    /// konstanter Iterator zum Element nach dem letzten Benutzername_str.
    auto end() const {
        return namen.cend();
    }

    /// Gibt Anzahl an Benutzernamen in der %Datei zurück.
    size_t anzahl() const {
        return namen.size();
    }

    /// Gibt #namen zurück
    std::vector <QString> const& get_namen() const {
        return namen;
    }

    Hineinschreiben& aktualisieren(); ///< Namen aktualisieren
    void reinschreiben(); ///< Meinen Benutzername_str in %Datei schreiben
    void herausnehmen(); ///< Meinen Benutzername_str aus %Datei entfernen

private:
    QString const& nutzername_str; ///< Mein Benutzername_str
    Datei const& file; ///< %Datei, in der die Namen stehen
    std::vector <QString> namen; ///< Alle Namen aus #file
    Datei_Mutex file_mtx; ///< Datei_Mutex für #file
};

#endif // HINEINSCHREIBEN_HPP
