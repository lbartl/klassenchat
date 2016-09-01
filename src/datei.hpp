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
// Dieser Header deklariert und definiert die Klasse Datei, mit der Dateien verwaltet werden können

#ifndef DATEI_HPP
#define DATEI_HPP

#include <boost/filesystem.hpp>
#include <fstream>
#include <sys/stat.h>

namespace fs = boost::filesystem;

/// Wegen einem Fehler in gcc5 std::exception
#if _GLIBCXX_USE_CXX11_ABI
using fstream_exc = std::exception;
#else
using fstream_exc = std::ios_base::failure;
#endif

/// Mit der Klasse Datei können Dateien verwaltet werden.
/**
 * Datei definiert Methoden für Dateien, wie das Löschen der %Datei, das Schreiben in die %Datei und andere.
 */
class Datei {
public:
    /// Konstruktor mit Pfad als C-String.
    Datei( char const*const pfad = "" ) :
        file( pfad ) {}

    /// Konstruktor mit Pfad als boost::filesystem::path.
    Datei( fs::path const& pfad ) :
        file( pfad.string() ) {}

    /// Expliziter Konstruktor mit Pfad als std::string.
    explicit Datei( std::string pfad ) :
        file( std::move( pfad ) ) {}

    /// Copy-Konstruktor.
    Datei( Datei const& other ) :
        file( other.file ) {}

    /// Move-Konstruktor.
    Datei( Datei&& other ) :
        file( std::move( other.file ) ) {}

    /// kopierender Zuweisungs-Operator.
    Datei& operator = ( Datei const& other ) {
        file = other.file;
        return *this;
    }

    /// bewegender Zuweisungs-Operator.
    Datei& operator = ( Datei&& other ) {
        file.swap( other.file );
        return *this;
    }

    /// std::string an Pfad anhängen.
    Datei& operator += ( std::string const& add ) {
        file += add;
        return *this;
    }

    /// C-String an Pfad anhängen.
    Datei& operator += ( char const*const add ) {
        file += add;
        return *this;
    }

    /// Zeichen an Pfad anhängen.
    Datei& operator += ( char const add ) {
        file += add;
        return *this;
    }

    /// Pfad als C-String zurückgeben.
    char const* getpath() const noexcept {
        return file.c_str();
    }

    /// Einen std::ifstream der %Datei zurückgeben.
    /**
     * Ein std::ifstream, der mit der %Datei geöffnet wurde wird zurückgegeben.
     * Das Flag std::ios::binary ist gesetzt und bei std::ios::badbit wird eine fstream_exc Exception geworfen.
     */
    std::ifstream istream() const {
        std::ifstream is ( file, std::ios::binary );
        is.exceptions( std::ios::badbit );
        return is;
    }

    /// Einen std::ofstream der %Datei zurückgeben.
    /**
     * @param app gibt an, ob dem Konstruktor des std::ofstream das Flag std::ios::app mitgegeben werden soll
     *
     * Ein std::ofstream, der mit der %Datei geöffnet wurde wird zurückgegeben.
     * Das Flag std::ios::binary ist gesetzt und bei std::ios::badbit und std::ios::failbit wird eine fstream_exc Exception geworfen.
     */
    std::ofstream ostream( bool app = false ) const {
        std::ofstream os ( file, app ? std::ios::app | std::ios::binary : std::ios::binary );
        os.exceptions( std::ios::badbit | std::ios::failbit );
        return os;
    }

    /// Prüfen, ob %Datei existiert.
    bool exist() const {
        struct stat buffer;
        return stat( file.c_str(), &buffer ) == 0;
    }

    /// %Datei erstellen.
    void touch() const {
        ostream();
    }

    /// Ans Ende der %Datei schreiben.
    /**
     * @param text der zu schreibende Text
     */
    void append( std::string const& text ) const {
        ostream( true ) << text << '\n';
    }

    /// Ans Ende der %Datei schreiben.
    /**
     * @param text der zu schreibende Text
     */
    void append( char const*const text ) const {
        ostream( true ) << text << '\n';
    }

    /// Dateiinhalt ersetzen.
    /**
     * @param text neuer Inhalt
     */
    void write( std::string const& text ) const {
        ostream() << text << '\n';
    }

    /// Dateiinhalt ersetzen.
    /**
     * @param text neuer Inhalt
     */
    void write( char const*const text ) const {
        ostream() << text << '\n';
    }

    /// Inhalt der %Datei löschen.
    void reset() const {
        ostream() << "";
    }
    /// %Datei entfernen.
    void remove() const {
        fs::remove( file );
    }

    /// Das erste Zeichen der %Datei zurückgeben.
    char readChar() const {
        return istream().get();
    }

    /// Die erste Zeile der %Datei einlesen.
    /**
     * @returns erste Zeile als std::string.
     */
    std::string readLine() const {
        std::string text;
        std::getline( istream(), text ); // Erste Zeile einlesen
        return text;
    }

    /// Gesamte %Datei einlesen.
    std::string readAll() const; // definitions.cpp

private:
    std::string file; ///< Dateipfad

friend std::string operator + ( std::string lhs, Datei const& rhs );
};

/// Dateipfad mit std::string verketten
inline std::string operator + ( std::string lhs, Datei const& rhs ) {
    return lhs += '\"' + rhs.file + '\"';
}

/// Dateipfad mit std::string zu neuem Pfad verketten
inline Datei operator + ( Datei lhs, std::string const& rhs ) {
    return lhs += rhs;
}

/// Dateipfad mit C-String zu neuem Pfad verketten
inline Datei operator + ( Datei lhs, char const*const rhs ) {
    return lhs += rhs;
}

/// Dateipfad mit einem Zeichen zu neuem Pfad verketten
inline Datei operator + ( Datei lhs, char const rhs ) {
    return lhs += rhs;
}

/// Dateipfad in Anführungszeichen ausgeben
inline std::ostream& operator << ( std::ostream& os, Datei const& file ) {
    return os << '\"' << file.getpath() << '\"'; // Pfad in Anführungszeichen ausgeben (da er Leerzeichen enthalten kann)
}

/// Dateipfad in Anführungszeichen einlesen
inline std::istream& operator >> ( std::istream& is, Datei& file ) {
    if ( is.get() != '\"' )
        is.setstate( std::ios::failbit );
    else {
        std::string str;
        std::getline( is, str, '\"' );
        file = Datei( std::move( str ) );
    }

    return is;
}

#endif // DATEI_HPP
