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

///\file
// Dieser Header definiert Funktionen für Pc-Nutzernamen

#ifndef PC_NUTZERNAME_HPP
#define PC_NUTZERNAME_HPP

#include <string>

class QWidget;

#ifdef WIN32
# include <locale>
# include <codecvt>
# include <lmcons.h>
# include <windows.h>
inline std::string get_pc_nutzername() {
    wchar_t name [UNLEN+1];
    DWORD size = UNLEN+1;
    return GetUserNameW( name, &size ) ? std::wstring_convert <std::codecvt_utf8_utf16 <wchar_t>>().to_bytes( name ) : "";
}
#else
# include <pwd.h>
# include <unistd.h>
/// Gibt meinen Pc-Nutzernamen zurück.
inline std::string get_pc_nutzername() {
    passwd* pw = getpwuid( geteuid() );
    return pw ? pw->pw_name : "";
}
#endif

/// Prüft, ob mein Pc-Nutzername verboten ist
bool pc_nutzername_verboten();
void verbotene_pc_nutzernamen_dialog( QWidget* parent = nullptr );

#endif // PC_NUTZERNAME_HPP
