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

#ifndef PRIVATCHATS_HPP
#define PRIVATCHATS_HPP

#include "nutzer.hpp"
#include <QMenu>

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

    void init( QMenu*const menuChats, QMenu*const menuAdmin, QAction*const ui_sep ) {
        this->menuChats = menuChats;
        this->menuAdmin = menuAdmin;
        this->ui_sep = ui_sep;
        chatfile_all = nutzer_ich.x_plum ? &chatfile_plum : &chatfile_norm;
        klassenchat();

        file_mtx_lock f_lock ( chatfile_all->file_mtx );
        chatfile_all->file.ostream( true ) << nutzer_ich.nutzername << " hat den Chat betreten\n";
    }

    void klassenchat() {
        lock_guard lock ( chatfile_mtx );
        menuAdmin->setEnabled( nutzer_ich.admin ); // Wenn man ein Admin ist einschalten, sonst ausschalten
        chatfile = chatfile_all;
    }

    bool imKlassenchat() {
        lock_guard lock ( chatfile_mtx );
        return chatfile == chatfile_all;
    }

    QString* getText() {
        lock_guard lock ( privatchats_mtx );

        if ( text.first ) {
            text.first = false;
            return &text.second;
        } else
            return nullptr;
    }

    std::string einlesen() {
        lock_guard lock ( chatfile_mtx );
        sharable_file_mtx_lock f_lock ( chatfile->file_mtx );
        return chatfile->file.readAll();
    }

    void schreibeInfo( char const*const info ) {
        file_mtx_lock f_lock ( chatfile_all->file_mtx );
        chatfile_all->file.append( info );
    }

    void entfernt( char const*const entferner ) {
        file_mtx_lock f_lock ( chatfile_all->file_mtx );
        chatfile_all->file.ostream( true ) << entferner << " hat " << nutzer_ich.nutzername << " entfernt\n";
    }

    void beenden() {
        lock_guard lock ( privatchats_mtx );
        privatchats.clear();
        file_mtx_lock f_lock ( chatfile_all->file_mtx );
        chatfile_all->file.ostream( true ) << nutzer_ich.nutzername << " hat den Chat verlassen\n";
    }

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

    void makeChat( std::string const& partner );
    void newChat( Datei chatdatei, size_t const partner_nummer );
    void changeChat( std::string const& partner );
    void schreibeNachricht( std::string const& nachricht );
    void flip_x_plum();
    void nutzerGeloescht( Nutzer const& nutzer );

private:
    ChatVerwaltung() = default;
    void neuerChat( Datei chatdatei, Nutzer const& partner );

    QMenu *menuChats {},
          *menuAdmin {};
    QAction* ui_sep {}; ///< Ein Seperator um "Neuer Privatchat..." von den Privatchats zu trennen
    std::pair <bool, QString> text {};

    struct Chatfile {
        Datei const file; ///< Chatdatei
        Datei_Mutex file_mtx { file }; ///< Datei_Mutex für #file

        Chatfile( Datei file ) :
            file( std::move( file ) )
        {}

        void remove() {
            file.remove();
            file_mtx.remove();
        }
    } chatfile_norm {"./verlauf.jpg"}, ///< Datei, in der der normale %Chat gespeichert ist
      chatfile_plum {"./baum.jpg"}; ///< Datei, in der der Plum-Chat gespeichert ist

    Chatfile *chatfile_all {}, ///< Zeiger auf Klassenchat-Datei (entweder #chatfile_norm oder #chatfile_plum)
             *chatfile {}; ///< Zeiger auf aktuelle Chatdatei (Privatchat oder Klassenchat)

    /// Mit dem struct Chataction können Aktionen für Privatchats verwaltet werden.
    struct Privatchat {
        QAction action; ///< QAction
        Chatfile file; ///< Chatdatei
        Nutzer const& partner; ///< Chatpartner

        /// Allgemeiner Konstruktor.
        /**
         * @param chatdatei Chatdatei
         * @param partner Chatpartner
         *
         * #action wird mit dem QString "&Chat mit <chatpartner>" und keinem Eltern-Objekt initialisiert.
         */
        Privatchat( Datei chatdatei, Nutzer const& partner ) :
            action( QString::fromStdString( "&Chat mit " + partner.nutzername ), nullptr ),
            file( std::move( chatdatei ) ),
            partner( partner )
        {}
    };

    /// Hier sind alle Privatchat%s gespeichert.
    std::list <Privatchat> privatchats {}; // Liste damit Zeiger gültig bleiben und nichts kopiert wird

    mutex chatfile_mtx {},
          privatchats_mtx {};

    void openChat( Privatchat*const chat ) {
        lock_guard lock ( chatfile_mtx );
        menuAdmin->setEnabled( false );
        chatfile = &chat->file;
    }
};

extern ChatVerwaltung& chat_verwaltung; ///< Referenz auf ChatVerwaltung::getInstance()

#endif // PRIVATCHATS_HPP
