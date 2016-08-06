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
// Dieser Header deklariert das Singleton AdminPass

#ifndef ADMINPASS_HPP
#define ADMINPASS_HPP

#include "cryptfile.hpp"
#include "nutzer.hpp"
#include <unordered_map>

/// Dieses Singleton verwaltet die Passwörter der #Chat::std_admins.
class AdminPass {
public:
    /// Gibt die einzige Instanz von AdminPass zurück
    static AdminPass& getInstance() {
        static AdminPass instance;
        return instance;
    }

    ///\cond
    AdminPass( AdminPass const& ) = delete;
    AdminPass& operator = ( AdminPass const& ) = delete;
    ///\endcond

    void setpass( std::string newpass ); ///< Setzt ein neues %Passwort für meinen Benutzernamen
    std::string getpass( std::string const& benutzername ) const; ///< %Passwort von einem Benutzernamen

    /// Ruft \ref getpass mit meinem Benutzernamen auf.
    std::string getpass() const {
        return getpass( nutzer_ich.nutzername );
    }

    /// Ruft fromFile() auf und gibt dann *this zurück.
    AdminPass& aktualisieren() {
        fromFile();
        return *this;
    }

private:
    AdminPass() { fromFile(); }
    void toFile(); ///< #allpass verschlüsseln und in #passfile schreiben
    void fromFile(); ///< #passfile entschlüsseln und in #allpass schreiben

    static constexpr char const* std_pass {"ichbinboss"}; ///< Standard-Passwort
    Cryptfile const passfile { "./pass.jpg", { 75, 45, 114, 30, 6, 203, 13, 102, 77, 155, 97, 100, 201, 170, 209, 178, 67, 102, 18, 218, 94, 106, 159, 126, 54, 65, 151, 91, 129, 107, 125, 102, 215, 96, 169, 18 } }; ///< verschlüsselte %Datei, in der die Passwörter stehen.
    Datei_Mutex file_mtx {"./pass.jpg"}; ///< Datei_Mutex für #passfile
    std::unordered_map <std::string, std::string> allpass {}; ///< Alle Benutzernamen und Passwörter unverschlüsselt
};

extern AdminPass& passwords; ///< Referenz auf AdminPass::getInstance()

#endif // ADMINPASS_HPP
