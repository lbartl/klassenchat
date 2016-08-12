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
// Dieser Header deklariert das Singleton ChatVerwaltung

#ifndef CHATVERWALTUNG_HPP
#define CHATVERWALTUNG_HPP

#include "nutzer.hpp"
#include <QMenu>

/// Dieses Singleton verwaltet alle Chats, also den Klassenchat und die Privatchats.
class ChatVerwaltung {
public:
    /// Gibt die einzige Instanz von ChatVerwaltung zurück
    static ChatVerwaltung& getInstance() {
        static ChatVerwaltung instance;
        return instance;
    }

    ///\cond
    ChatVerwaltung( ChatVerwaltung const& ) = delete;
    ChatVerwaltung& operator = ( ChatVerwaltung const& ) = delete;
    ///\endcond

    /// Muss aufgerufen werden, bevor ChatVerwaltung benutzt wird
    /**
     * @param menuChats Zeiger auf das QMenu "Chats" im Chat-Fenster
     * @param menuAdmin Zeiger auf das QMenu "Admin" im Chat-Fenster
     * @param ui_sep Ein Seperator um "Neuer Privatchat..." von den Privatchats zu trennen
     *
     * Diese Methode initialisiert ChatVerwaltung.
     * #chatfile_all wird festgelegt, klassenchat() wird aufgerufen
     * und in #chatfile_all wird geschrieben, dass ich den %Chat betreten habe.
     */
    void init( QMenu*const menuChats, QMenu*const menuAdmin, QAction*const ui_sep ) {
        this->menuChats = menuChats;
        this->menuAdmin = menuAdmin;
        this->ui_sep = ui_sep;
        chatfile_all = nutzer_ich.x_plum ? &chatfile_plum : &chatfile_norm;
        klassenchat();

        file_mtx_lock f_lock ( chatfile_all->file_mtx );
        chatfile_all->file.ostream( true ) << nutzer_ich.nutzername << " hat den Chat betreten\n";
    }

    /// In den Klassenchat wechseln
    void klassenchat() {
        lock_guard lock ( chatfile_mtx );
        menuAdmin->setEnabled( nutzer_ich.admin ); // Wenn man ein Admin ist einschalten, sonst ausschalten
        chatfile = chatfile_all;
    }

    /// Überprüft, ob ich im Klassenchat bin
    bool imKlassenchat() {
        lock_guard lock ( chatfile_mtx );
        return chatfile == chatfile_all;
    }

    /// Wenn etwas in #text steht, wird #text zurückgegeben, ansonsten wird nullptr zurückgegeben
    QString* getText() {
        lock_guard lock ( privatchats_mtx );

        if ( text.first ) {
            text.first = false;
            return &text.second;
        } else
            return nullptr;
    }

    /// Gibt Inhalt von #chatfile zurück
    std::string einlesen() {
        lock_guard lock ( chatfile_mtx );
        sharable_file_mtx_lock f_lock ( chatfile->file_mtx );
        return chatfile->file.readAll();
    }

    /// Schreibt eine Information in #chatfile_all
    void schreibeInfo( char const*const info ) {
        file_mtx_lock f_lock ( chatfile_all->file_mtx );
        chatfile_all->file.append( info );
    }

    /// In #chatfile_all schreiben, dass ich entfernt wurde
    void entfernt( char const*const entferner ) {
        file_mtx_lock f_lock ( chatfile_all->file_mtx );
        chatfile_all->file.ostream( true ) << entferner << " hat " << nutzer_ich.nutzername << " entfernt\n";
    }

    /// In #chatfile_all schreiben, dass ich den %Chat verlassen habe
    void beenden() {
        lock_guard lock ( privatchats_mtx );
        privatchats.clear();
        file_mtx_lock f_lock ( chatfile_all->file_mtx );
        chatfile_all->file.ostream( true ) << nutzer_ich.nutzername << " hat den Chat verlassen\n";
    }

    /// Inhalt von #chatfile_all löschen
    void reset() {
        if ( chatfile_all ) {
            file_mtx_lock f_lock ( chatfile_all->file_mtx );
            chatfile_all->file.reset();
        } else {
            file_mtx_lock l1 ( chatfile_norm.file_mtx ),
                          l2 ( chatfile_plum.file_mtx );
            chatfile_norm.file.reset();
            chatfile_plum.file.reset();
        }
    }

    void makeChat( std::string const& partner ); ///< Einen neuen %Privatchat erstellen
    void newChat( Datei chatdatei, size_t const partner_nummer ); ///< Einen neuen %Privatchat öffnen
    void changeChat( std::string const& partner ); ///< %Privatchat öffnen oder erstellen
    void schreibeNachricht( std::string const& nachricht ); ///< Eine Nachricht in #chatfile schreiben
    void flip_x_plum(); ///< Sollte nach NutzerVerwaltung::flip_x_plum() aufgerufen werden
    void nutzerGeloescht( Nutzer const& nutzer ); ///< Ein %Nutzer ist nicht mehr im %Chat, überprüfen, ob dieser einen %Privatchat mit mir hatte

private:
    ChatVerwaltung() = default;
    void neuerChat( Datei chatdatei, Nutzer const& partner ); ///< Einen neues Objekt von Privatchat erstellen und in #privatchats speichern

    QMenu *menuChats {}, ///< Zeiger auf das QMenu "Chats" im Chat-Fenster
          *menuAdmin {}; ///< Zeiger auf das QMenu "Admin" im Chat-Fenster
    QAction* ui_sep {}; ///< Ein Seperator um "Neuer Privatchat..." von den Privatchats zu trennen
    std::pair <bool, QString> text {}; ///< Text für einen Dialog, wenn ein %Privatchat gelöscht wurde

    /// Mit dem struct Chatfile kann eine Chatdatei verwaltet werden.
    struct Chatfile {
        Datei const file; ///< Chatdatei
        Datei_Mutex file_mtx { file }; ///< Datei_Mutex für #file

        /// Konstruktor.
        Chatfile( Datei file ) :
            file( std::move( file ) )
        {}

        /// #file und #file_mtx löschen
        void remove() {
            file.remove();
            file_mtx.remove();
        }
    };

    static Chatfile chatfile_norm, chatfile_plum; // chatverwaltung.cpp

    Chatfile *chatfile_all {}, ///< Zeiger auf Klassenchat-Datei (entweder #chatfile_norm oder #chatfile_plum)
             *chatfile {}; ///< Zeiger auf aktuelle Chatdatei (%Privatchat oder Klassenchat)

    /// Mit dem struct Privatchat können Privatchats verwaltet werden.
    struct Privatchat {
        QAction action; ///< QAction
        Chatfile file; ///< Chatdatei
        Nutzer const& partner; ///< Chatpartner

        /// Konstruktor.
        /**
         * @param chatdatei Chatdatei
         * @param partner Chatpartner
         *
         * #action wird mit dem QString "&Chat mit <partner>" und keinem Eltern-Objekt initialisiert.
         */
        Privatchat( Datei chatdatei, Nutzer const& partner ) :
            action( QString::fromStdString( "&Chat mit " + partner.nutzername ), nullptr ),
            file( std::move( chatdatei ) ),
            partner( partner )
        {}
    };

    /// Hier sind alle Privatchat%s gespeichert.
    std::list <Privatchat> privatchats {}; // Liste damit Zeiger gültig bleiben und nichts kopiert wird

    mutex chatfile_mtx {}, ///< Mutex für die Synchronisation von #chatfile
          privatchats_mtx {}; ///< Mutex für die Synchronisation von #privatchats

    /// In einen %Privatchat wechseln
    void openChat( Privatchat*const chat ) {
        lock_guard lock ( chatfile_mtx );
        menuAdmin->setEnabled( false );
        chatfile = &chat->file;
    }
};

extern ChatVerwaltung& chat_verwaltung; ///< Referenz auf ChatVerwaltung::getInstance()

#endif // CHATVERWALTUNG_HPP
