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

// Dieser Header deklariert die Klasse Chat

#ifndef CHAT_HPP
#define CHAT_HPP

#include "ui_chat.h"
#include "adminpass.hpp"
#include <bitset>
#include <forward_list>

/// Wichtigste Klasse des Chats.
/**
 * Beim Aufruf des Programms wird, nach dem Passwort-Dialog, ein Objekt der Klasse Chat erzeugt, das die ganze Verwaltung des Chats übernimmt.
 * Dieses stellt gleichzeitig auch das Haupt-Fenster da.
 */
class Chat : public QMainWindow {
    Q_OBJECT

public:
    /// Allgemeiner Konstruktor.
    explicit Chat( bool plum, QWidget* parent = nullptr );

    ///\cond
    // gelöscht, damit keine Compiler-Warnung (wäre auch implizit gelöscht)
    Chat( Chat const& ) = delete;
    Chat& operator = ( Chat const& ) = delete;
    ///\endcond

    /// Einen neuen Privatchat erstellen.
    void make_chat( Nutzer const& partner ); // privatchats.cpp

#ifndef OBERADMIN
# define OBERADMIN "LukasB"
#endif
    /// Der Oberadmin des Chats. Er kann am meisten machen, wie zum Beispiel Objekte der Klasse Admin erstellen.
    /**
     * Der Oberadmin kann beim Kompilieren in der %Datei "config/oberadmin" festgelegt werden.
     * Es darf nur der Name des Oberadmins und nichts anderes in der %Datei stehen!
     *
     * Wenn die %Datei nicht existiert, wird "LukasB" als Oberadmin genommen.
     */
    static constexpr char const (&oberadmin)[sizeof(OBERADMIN)] { OBERADMIN }; // Referenz des Arrays und kein Zeiger, damit std::end() in aktualisieren.cpp funktioniert

#ifndef STD_ADMINS
# define STD_ADMINS { oberadmin, "Oberlusche", "Patiboy", "Jaguar" }
#endif
#define STD_ADMINS_COUNT std::initializer_list <char const*> ( STD_ADMINS ).size() // Wie viele Standard-Admins es gibt
    /// Standard-Administratoren. Sie sind automatisch beim Anmelden Admins und haben ein individuelles %Passwort, siehe AdminPass.
    /**
      * Der #oberadmin ist automatisch ein Standard-Admin.
      * Die anderen Standard-Admins können beim Kompilieren in der %Datei "config/std_admins", getrennt durch Whitespaces, festgelegt werden.
      *
      * Wenn die %Datei nicht existiert, werden "Oberlusche", "Patiboy" und "Jaguar" als Standard-Admins genommen.
      */
    static constexpr std::array <char const*, STD_ADMINS_COUNT> std_admins { STD_ADMINS };
#undef STD_ADMINS_COUNT
#undef STD_ADMINS

protected:
    /// Statt dem Schließen #x_close setzen.
    void closeEvent( QCloseEvent* event ) override final; // chat.cpp

private:
    // statische Member in chat.cpp definiert
    static Datei const chatfile_norm, chatfile_plum, lockfile_norm, lockfile_plum;

    bool const x_plum_anfang; ///< x_plum beim Chat öffnen (je nach Passwort)

    /// Enumerator für #flags.
    enum {
        locked, ///< Zeigt für #oberadmin an, ob Admins ihn entfernen dürfen
        std_admin, ///< Zeigt an ob man einer der #std_admins ist
        x_oberadmin, ///< Zeigt an ob man der #oberadmin ist
        chatall, ///< Zeigt an ob man im Klassenchat ist
        x_close, ///< Zeigt an ob der %Chat geschlossen werden soll
        x_restart, ///< Zeigt an ob der %Chat neugestartet werden soll
        x_main, ///< Zeigt an ob man sich auf der Chat-Seite befindet
        x_reload, ///< Zeigt an ob der Chat-Verlauf neu geladen werden soll
        COUNT ///< Anzahl an Flags
    };

    std::bitset <COUNT> flags {}; ///< Flags des Chats
    Ui::Chat ui {}; ///< UI des Chats
    QAction* ui_sep {}; ///< Ein Seperator um "Neuer Privatchat..." von den Privatchats zu trennen
    std::string inhalt {}; ///< Aktueller Inhalt von #chatfile, benutzt von aktualisieren_thread() und verlauf_up()
    Datei terminatefile {}, ///< %Datei, die, wenn sie existiert, anzeigt, dass jemand mich entfernt hat (zugewiesen in start())
          infofile {}, ///< %Datei, die, wenn sie existiert, anzeigt, dass etwas mit mir geschehen soll (zugewiesen in start())
          checkfile {}; ///< %Datei, mit der überprüft werden kann ob man noch im Chat ist (zugewiesen in start())
    Datei const *chatfile_all, ///< Zeiger auf Klassenchat-Datei (entweder #chatfile_norm oder #chatfile_plum)
                *lockfile;     ///< Zeiger auf eigenes lockfile  (entweder #lockfile_norm oder #lockfile_plum)

    struct : public std::atomic <Datei const*> {
        ///\cond
        Datei const* operator -> () {
            return *this;
        }

        auto& operator = ( Datei const*const other ) {
            this -> store( other );
            return *this;
        }
        ///\endcond
    } chatfile {}; ///< Zeiger auf aktuelle Chatdatei (Privatchat oder Klassenchat)

    /// Mit dem struct Chataction können Aktionen für Privatchats verwaltet werden.
    struct Chataction {
        QAction action; ///< QAction
        Datei const datei; ///< Chatdatei
        std::string const partner; ///< Chatpartner

        /// Allgemeiner Konstruktor.
        /**
         * @param file Chatdatei
         * @param chatpartner Chatpartner
         *
         * #action wird mit dem QString "&Chat mit <chatpartner>" und keinem Eltern-Objekt initialisiert.
         */
        Chataction( Datei file, std::string chatpartner ) :
            action( QString::fromStdString( "&Chat mit " + chatpartner ), nullptr ),
            datei( std::move( file ) ),
            partner( std::move( chatpartner ) )
        {}
    };

    /// Hier sind alle Chataction%s gespeichert.
    std::forward_list <Chataction> chats_ac {}; // forward_list damit Zeiger gültig bleiben und nichts kopiert wird

    /// Mit der Klasse UiThing können Threads Änderungen an #ui an pruefen_main() weitergeben.
    /**
     * Für einen Konstruktor-Aufruf für first() oder second() sollte Placement-new benutzt werden.
     * Für einen Destruktor-Aufruf für first() \b und second() sollte destruct() benutzt werden.
     */
    class UiThing {
    public:
        /// Zeigt was geschehen soll.
        enum Typ : uint_fast8_t {
            nichts, ///< Nichts soll geschehen
            aktualisieren, ///< Der Chatverlauf soll aktualisiert werden, mit first()=Position in #inhalt (size_t)
            terminate, ///< Den %Chat beenden
            entfernt, ///< Ich wurde entfernt, nötige Informationen in #chatfile_all schreiben, first()=Entferner (std::string)
            Warnung, ///< Eine Warnung soll angezeigt werden
            ToAdmin, ///< Mich zum %Admin machen
            FromAdmin, ///< Mich zum normalen %Nutzer machen
            Dialog, ///< Ein SimpleDialog soll angezeigt werden, mit first()=Titel (QString) und second()=Text (QString)
            Privatchat ///< new_chat() soll aufgerufen werden, mit first()=Chatdatei (Datei) und second()=Chatpartner (std::string)
        };

        mutex mtx {}; ///< Mutex für die Synchronisation

        UiThing() = default;

        /// Destruktor. Ruft destruct() auf.
        ~UiThing() {
            destruct();
        }

        ///\cond
        UiThing( UiThing const& ) = delete;
        UiThing& operator = ( UiThing const& ) = delete;
        ///\endcond

        /// Gibt Zeiger auf erstes Objekt zurück, siehe #entfernt, #Dialog und #Privatchat.
        /**
         * \code
         * return reinterpret_cast <T*> ( speicher );
         * \endcode
         */
        template <typename T = void>
        T* first() {
            return reinterpret_cast <T*> ( speicher );
        }

        /// Gibt Zeiger auf zweites Objekt zurück, siehe #Dialog und #Privatchat.
        /**
         * \code
         * return reinterpret_cast <T*> ( speicher + size_one );
         * \endcode
         */
        template <typename T = void>
        T* second() {
            return reinterpret_cast <T*> ( speicher + size_one );
        }

        /// Gibt #typ zurück.
        Typ getTyp() {
            return typ;
        }

        /// Setzte #typ neu.
        /**
         * @param lock Lock für #cond
         * @param neuer_typ Der neue #Typ
         *
         * #cond wartet bis #typ == #nichts, dann wird #typ neu gesetzt.
         */
        void newTyp( unique_lock& lock, Typ const neuer_typ ) {
            cond.wait( lock, [this] () { return typ == nichts; } );
            typ = neuer_typ;
        }

        /// Ruft newTyp() auf mit neu angelegtem #unique_lock (initialisiert mit #mtx).
        void newTyp( Typ const neuer_typ ) {
            unique_lock lock ( mtx );
            newTyp( lock, neuer_typ );
        }

        /// Destruktoren aufrufen, #typ auf #nichts setzen und cond.notify_one() aufrufen.
        void destruct() {
            switch ( typ ) {
            case entfernt:
                first <std::string>() -> ~basic_string();
                break;
            case Dialog:
                first <QString>() -> ~QString();
                second <QString>() -> ~QString();
                break;
            case Privatchat:
                first <Datei>() -> ~Datei();
                second <std::string>() -> ~basic_string();
            default:
                break;
            }

            typ = nichts;
            cond.notify_one();
        }
    private:
        static constexpr size_t size_one = std::max({ sizeof(QString), sizeof(std::string), sizeof(Datei) }); ///< Maximale Größe eines Objekts
        unsigned char speicher [2*size_one]; ///< Speicher für first() und second()
        Typ typ = nichts; ///< Zeigt was geschehen soll
        condition_variable cond {}; ///< Condition-Variable für die Synchronisation
    } nextUiThing {}; ///< Einziges Objekt von UiThing

    mutex nutzer_mtx {}, ///< Mutex, die nutzer_thread(), plum_chat() und in bestimmten Situationen pruefen_thread() sperren
          pruefen_mtx {}, ///< Mutex, die pruefen_thread(), plum_chat() und warnung_send() sperren
          chats_ac_mtx {}; ///< Mutex für die Synchronisation von #chats_ac

    Datei_Mutex lockfile_mtx { *lockfile }, ///< Datei_Mutex, die das Schreiben von #lockfile kontrolliert
                chatfile_all_mtx { *chatfile_all }; ///< Datei_Mutex, die das Schreiben von #chatfile_all kontrolliert

    // aktualisieren.cpp
    void verlauf_up( size_t pos );
    void main_thread();

    // dialog.cpp
    void entfernen( std::string const& name = "" );
    void hilfe_anz();
    void nutzer_anz();
    void personal_op( std::string const& partner = "" );
    void info_open( std::string const& an = "" );
    void openAdminPass();

    // kommandos.cpp
    void resetcv();
    void allt();
    void warnung_send();
    void plum_chat();

    // privatchats.cpp
    bool exist_chat( std::string const& partner );
    void new_chat( Datei dateichat, std::string partner );
    void open_chat( Datei const*const chatdatei );
    void klassenchat();
    void ch_chat( std::string const& partner = "" );
    void check_all_chats();

    // pruefen.cpp
    bool pruefen_main();

    // senden.cpp
    void senden_pruef();

    // start.cpp
    void start();
    void passwort();
    void start2();
    void stop();

    // threads.cpp
    void start_threads();
    void stop_threads();
    void aktualisieren_thread();
    void nutzer_thread();
    void pruefen_thread();

private slots:
    void vordergrund( bool const status ); // chat.cpp
    void set_pass( std::string newpass ); // kommandos.cpp
};

#endif // CHAT_HPP
