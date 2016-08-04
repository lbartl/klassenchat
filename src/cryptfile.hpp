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

// Dieser Header deklariert die Klasse Cryptfile

#ifndef CRYPTFILE_HPP
#define CRYPTFILE_HPP

#include "datei.hpp"

/// Mit der Klasse Cryptfile können verschlüsselte Dateien verwaltet werden.
/**
 * Cryptfile bietet Methoden zum Schreiben von normalen Text, der dann verschlüsselt wird, in eine %Datei
 * und zum Lesen des Inhalts, der vorher entschlüsselt wird.
 */
class Cryptfile {
public:
    /// Allgemeiner Konstruktor.
    explicit Cryptfile( Datei datei, std::vector <uint8_t> schluessel );

    /// Prüfen ob %Datei existiert.
    bool exist() const {
        return file.exist();
    }

    /// %Datei erstellen.
    void touch() const {
        file.touch();
    }

    /// Inhalt der %Datei löschen.
    void reset() const {
        file.reset();
        keyat = 0;
    }

    /// %Datei entfernen.
    void remove() const {
        file.remove();
        keyat = 0;
    }

    /// %Datei entschlüsseln.
    std::string decrypt() const;

    /// Ans Ende der %Datei schreiben.
    void append( std::string text ) const;

    /// Text veschlüsseln und in %Datei schreiben.
    /**
     * @param plaintext normaler Text, der dann verschlüsselt in %Datei geschrieben wird
     */
    void encrypt( std::string plaintext ) const {
        reset();
        append( std::move( plaintext ) );
    }

private:
    Datei const file; ///< verschlüsselte %Datei
    std::vector <uint8_t> key; ///< Schlüssel

    ///\cond
    size_t mutable keyat = 0; // Nächster Schlüssel
    ///\endcond
};

#endif // CRYPTFILE_HPP
