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

///\file
// Dieser Header deklariert globale Funktionen, Funktoren und Variablen

#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <QRegExp>

/// Prüfen ob Container einen Wert enthält.
/**
 * @param container Container
 * @param value Wert
 *
 * Prüft ob value in container vorhanden ist.
 */
template <typename Container, typename T>
constexpr bool enthaelt( Container const& container, T const& value ) {
    return std::find( std::begin( container ), std::end( container ), value ) != std::end( container );
}

/// Prüfen ob Cotainer nur aus Elementen besteht, die gleich einem Wert sind.
/**
 * @param first Iterator des ersten Elements
 * @param last Iterator zum Element nach dem letzten Element
 * @param value Wert
 *
 * Prüft ob [first, last) nur aus Elementen besteht, die gleich wie value sind.
 * Wenn der Container leer ist, wird true zurückgegeben.
 */
template <typename InputIt, typename T>
constexpr bool enthaelt_nur( InputIt first, InputIt last, T const& value ) {
    return std::all_of( first, last, [&value] ( T const& currval ) { return currval == value; } );
}

/// Prüfen ob die Buchstaben von zwei QStrings gleich sind, sie aber evtl. eine andere Groß-/Kleinschreibung haben.
/**
 * @param a erster String
 * @param b zweiter String
 * @returns true, wenn a und b von den Buchstaben (nicht unbedingt von der Groß-/Kleinschreibung!) gleich sind, sonst false.
 */
inline bool caseInsEqual( QString const& a, QString const& b ) {
    return a.compare( b, Qt::CaseInsensitive ) == 0;
}

/// Funktor für \ref caseInsEqual
struct caseInsEqualFunc {
    QString const& a; ///< gespeicherter QString, der verglichen wird

    /// Mit einem anderen QString vergleichen, ruft \ref caseInsEqual auf.
    bool operator () ( QString const& b ) {
        return caseInsEqual( a, b );
    }
};

extern const QRegExp regex_nutzername; ///< Legt fest, wie ein Benutzername aussehen soll

#endif // FUNCTIONS_HPP
