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

// Diese Datei ruft der Nutzer auf, und sie ruft dann die anderen Dateien auf

#include "chat.hpp"
#include "passwort.hpp"
#include "filesystem.hpp"
#include "klog.hpp"

using std::clog;
using std::cerr;

#ifdef DEBUG

void messageOutput( QtMsgType type, QMessageLogContext const& context, QString const& msg ) { // Für Debugging
    ios_base::sync_with_stdio( false );

    switch ( type ) {
    case QtDebugMsg:
        clog << msg.toStdString() << endl;
        break;
    case QtInfoMsg:
        clog << "Info: " << msg.toStdString() << endl;
        break;
    case QtWarningMsg:
        clog << "Warnung: "  << msg.toStdString() << endl;
        break;
    case QtCriticalMsg:
        cerr << "Schwerwiegender Fehler: " << msg.toStdString() << " (" << context.file << ':' << context.line << ", " << context.function << ")\n";
        break;
    case QtFatalMsg:
        cerr << "Schwerwiegender Fehler: " << msg.toStdString() << " (" << context.file << ':' << context.line << ", " << context.function << ")\n";
        abort();
    }
}

#define TRY_RELEASE
#define CATCH_RELEASE

#else // DEBUG

void messageOutput( QtMsgType type, QMessageLogContext const&, QString const& msg ) { // Für Release
    ios_base::sync_with_stdio( false );

    switch ( type ) {
    case QtDebugMsg: // wird nicht angezeigt
        break;
    case QtInfoMsg:
        clog << "Info: " << msg.toStdString() << endl;
        break;
    case QtWarningMsg:
        clog << "Warnung: " << msg.toStdString() << endl;
        break;
    case QtCriticalMsg:
        cerr << "Schwerwiegender Fehler: " << msg.toStdString() << '\n';
        break;
    case QtFatalMsg:
        cerr << "Schwerwiegender Fehler: " << msg.toStdString() << '\n';
        abort();
    }
}

#define TRY_RELEASE try
#define CATCH_RELEASE catch (...) { throw; }

#endif // DEBUG

int main( int argc, char* argv[] ) TRY_RELEASE {
    QApplication app ( argc, argv );
    qInstallMessageHandler( messageOutput );

    if ( ! Datei("./icon.ico").exist() ) // Die einzige wichtige Datei im Chat-Verzeichnis ist das Icon, alles andere wird automatisch wieder erstellt
        qFatal("Icon nicht gefunden!");

    bool plum;

    { // Passwort-Dialog
        Passwort pass;
        pass.exec();

        std::string const& passein { pass.getpass() };

        if ( passein == "chat" ) // Plum-Chat
            plum = true;
        else if ( passein == "baum" ) // Normaler Chat
            plum = false;
        else {
            qCritical("Falsches Passwort!");
            return EXIT_FAILURE;
        }
    }

    klog("Starte Chat...");

    Chat w ( plum ); // Chat starten
    w.show(); // Chat-Fenster anzeigen

    return app.exec();
} CATCH_RELEASE // Alle Exceptions hier fangen, damit Destruktoren aufgerufen werden
