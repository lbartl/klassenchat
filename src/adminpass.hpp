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

// Dieser Header deklariert die Klasse AdminPass

#ifndef ADMINPASS_HPP
#define ADMINPASS_HPP

#include "cryptfile.hpp"
#include <unordered_map>

/// Mit der Klasse AdminPass können die Passwörter der #Chat::std_admins verwaltet werden. Einziges Objekt ist #Chat::passwords.
class AdminPass
{
public:
    static constexpr char const* std_pass {"ichbinboss"}; ///< Standard-Passwort

    explicit AdminPass( std::string const& benutzername ); ///< Allgemeiner Konstruktor

    void setpass( std::string newpass ); ///< Setzt ein neues %Passwort für meinen Benutzernamen
    std::string getpass( std::string const& benutzername ) const; ///< %Passwort von einem Benutzernamen

    /// Ruft \ref getpass mit meinem Benutzernamen auf.
    std::string getpass() const {
        return getpass( nutzername );
    }

    /// Ruft fromFile() auf und gibt dann *this zurück.
    AdminPass& aktualisieren() {
        fromFile();
        return *this;
    }

private:
    std::string const& nutzername; ///< Mein Benutzername
    Cryptfile const passfile; ///< verschlüsselte %Datei, in der die Passwörter stehen
    Datei_Mutex file_mtx; ///< Datei_Mutex für #passfile
    std::unordered_map <std::string, std::string> allpass {}; ///< Alle Benutzernamen und Passwörter unverschlüsselt

    void toFile(); ///< #allpass verschlüsseln und in #passfile schreiben
    void fromFile(); ///< #passfile entschlüsseln und in #allpass schreiben
};

#endif // ADMINPASS_HPP
