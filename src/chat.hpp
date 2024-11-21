/* Copyright (C) 2015,2016,2018 Lukas Bartl
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

#include <QVariant>
#include "ui_chat.h"
#include "adminpass.hpp"
#include <bitset>
#include <tuple>
#include <string_view>
#include <any>

/// Wichtigste Klasse des Chats.
/**
 * Beim Aufruf des Programms wird, nach dem Passwort-Dialog, ein Objekt der Klasse Chat erzeugt, das die ganze Verwaltung des Chats übernimmt.
 * Dieses stellt gleichzeitig auch das Haupt-Fenster da.
 */
class Chat : public QMainWindow
{
    Q_OBJECT

public:
    explicit Chat( bool const x_plum, QWidget* parent = nullptr ); ///< Konstruktor.

    ///\cond
    // gelöscht, damit keine Compiler-Warnung (wäre auch implizit gelöscht)
    Chat( Chat const& ) = delete;
    Chat& operator = ( Chat const& ) = delete;
    ///\endcond

#ifndef OBERADMIN
# define OBERADMIN "LukasB"
#endif
    /// Der Oberadmin des Chats. Er kann am meisten machen, wie zum Beispiel Objekte der Klasse Admin_anz erstellen.
    /**
     * Der Oberadmin kann beim Kompilieren in der %Datei "config/oberadmin" festgelegt werden.
     * Es darf nur der Name des Oberadmins und nichts anderes in der %Datei stehen!
     *
     * Wenn die %Datei nicht existiert, wird "LukasB" als Oberadmin genommen.
     */
    static constexpr std::string_view oberadmin { OBERADMIN }; // Referenz des Arrays und kein Zeiger, damit std::end() in aktualisieren.cpp funktioniert

#ifndef STD_ADMINS
# define STD_ADMINS { oberadmin, "Oberlusche", "Patiboy", "Jaguar" }
#endif
#define STD_ADMINS_COUNT std::initializer_list <std::string_view> ( STD_ADMINS ).size() // Wie viele Standard-Admins es gibt
    /// Standard-Administratoren. Sie sind automatisch beim Anmelden Admins und haben ein individuelles %Passwort, siehe AdminPass.
    /**
      * Der #oberadmin ist automatisch ein Standard-Admin.
      * Die anderen Standard-Admins können beim Kompilieren in der %Datei "config/std_admins", getrennt durch Whitespaces, festgelegt werden.
      *
      * Wenn die %Datei nicht existiert, werden "Oberlusche", "Patiboy" und "Jaguar" als Standard-Admins genommen.
      */
    static constexpr std::array <std::string_view, STD_ADMINS_COUNT> std_admins { STD_ADMINS };
#undef STD_ADMINS_COUNT
#undef STD_ADMINS

protected:
    /// Statt dem Schließen #x_close setzen.
    void closeEvent( QCloseEvent* event ) override final; // chat.cpp

private:
    static inline Datei const lockfile_norm {"./lock"}, ///< %Datei, die existiert, wenn jemand im normalen %Chat ist
                              lockfile_plum {"./baum"}; ///< %Datei, die existiert, wenn jemand im Plum-Chat ist

    bool const x_plum_anfang; ///< x_plum beim Öffnen des Chats (je nach %Passwort)

    /// Enumerator für #flags.
    enum {
        locked, ///< Zeigt für #oberadmin an, ob Admins ihn entfernen dürfen
        std_admin, ///< Zeigt an, ob man einer der #std_admins ist
        x_oberadmin, ///< Zeigt an, ob man der #oberadmin ist
        x_close, ///< Zeigt an, ob der %Chat geschlossen werden soll
        x_restart, ///< Zeigt an, ob der %Chat neugestartet werden soll
        x_main, ///< Zeigt an, ob man sich auf der Chat-Seite befindet
        x_reload, ///< Zeigt an, ob der Chat-Verlauf neu geladen werden soll
        COUNT ///< Anzahl an Flags
    };

    std::bitset <COUNT> flags {}; ///< Flags des Chats
    Ui::Chat ui {}; ///< UI des Chats
    std::string inhalt {}; ///< Aktueller Inhalt von ChatVerwaltung::einlesen(), benutzt von aktualisieren_thread() und verlauf_up()
    Datei terminatefile {}, ///< %Datei, die, wenn sie existiert, anzeigt, dass jemand mich entfernt hat (zugewiesen in start2())
          infofile {}, ///< %Datei, die, wenn sie existiert, anzeigt, dass etwas mit mir geschehen soll (zugewiesen in start2())
          checkfile {}; ///< %Datei, mit der überprüft werden kann ob man noch im %Chat ist (zugewiesen in start2())
    Datei const* lockfile; ///< Zeiger auf eigenes lockfile  (entweder #lockfile_norm oder #lockfile_plum)

    /// Mit der Klasse UiThing können Threads Änderungen an #ui an pruefen_main() weitergeben.
    /**
     * Für einen Konstruktor-Aufruf für first() oder second() sollte Placement-new benutzt werden.
     * Für einen Destruktor-Aufruf für first() \b und second() sollte destruct() benutzt werden.
     */
    class UiThing {
    public:
        /// Zeigt was geschehen soll.
        enum Typ {
            nichts, ///< Nichts soll geschehen
            beendet, ///< UiThing soll keine neuen Typen mehr annehmen
            aktualisieren, ///< Der Chatverlauf soll aktualisiert werden, mit first()=Position in #inhalt (size_t)
            terminate, ///< Den %Chat beenden
            entfernt, ///< Ich wurde entfernt, ChatVerwaltung::entfernt() aufrufen, first()=Entferner (std::string)
            Warnung, ///< Eine Warnung soll angezeigt werden
            ToAdmin, ///< Mich zum %Admin machen
            FromAdmin, ///< Mich zum normalen %Nutzer machen
            Dialog, ///< Ein SimpleDialog soll angezeigt werden, mit first()=Titel (QString) und second()=Text (QString)
            Privatchat ///< ChatVerwaltung::newChat() soll aufgerufen werden, mit first()=Chatdatei (Datei) und second()=Chatpartner (size_t)
        };

        ///\cond
        UiThing() = default;
        UiThing( UiThing const& ) = delete;
        UiThing& operator = ( UiThing const& ) = delete;
        ///\endcond

        auto get() {
            lock_guard lock (mtx);
            std::tuple tup {typ, std::move(first), std::move(second)};
            typ = nichts;
            cond.notify_one();
            return tup;
        }

        template <typename T1, typename T2>
        void set(Typ const neuer_typ, T1 erstes, T2 zweites) {
            unique_lock lock (mtx);
            cond.wait( lock, [this] () { return typ == nichts || typ == beendet; } );

            if (typ == nichts) {
                typ = neuer_typ;
                first = std::move(erstes);
                second = std::move(zweites);
            }
        }

        template <typename T>
        void set(Typ const neuer_typ, T erstes) {
            unique_lock lock (mtx);
            cond.wait( lock, [this] () { return typ == nichts || typ == beendet; } );

            if (typ == nichts) {
                typ = neuer_typ;
                first = std::move(erstes);
            }
        }

        void set(Typ const neuer_typ) {
            unique_lock lock (mtx);
            cond.wait( lock, [this] () { return typ == nichts || typ == beendet; } );

            if (typ == nichts)
                typ = neuer_typ;
        }

        void start() {
            typ = nichts;
        }

        /// #typ auf #beendet setzen und cond.notify_all() aufrufen.
        void beenden() {
            typ = beendet;
            cond.notify_all();
        }

    private:
        Typ typ = nichts; ///< Zeigt was geschehen soll
        std::any first {}, ///< Erstes Objekt
                 second {}; ///< Zweites Objekt
        mutex mtx {}; ///< Mutex für die Synchronisation
        condition_variable cond {}; ///< Condition-Variable für die Synchronisation

    } nextUiThing {}; ///< Einziges Objekt von UiThing

    mutex nutzer_mtx {}, ///< Mutex, die nutzer_thread(), plum_chat() und in bestimmten Situationen pruefen_thread() sperren
          pruefen_mtx {}, ///< Mutex, die pruefen_thread(), plum_chat() und warnung_send() sperren
          inhalt_mtx {}; ///< Mutex für #inhalt

    Datei_Mutex lockfile_mtx { *lockfile }; ///< Datei_Mutex, die das Schreiben von #lockfile kontrolliert

    // aktualisieren.cpp
    void verlauf_up( size_t pos );

    // dialog.cpp
    void entfernen( std::string_view const& name = "" );
    void hilfe_anz();
    void nutzer_anz();
    void personal_op( std::string const& partner = "" );
    void info_open( std::string const& an = "" );
    void openAdminPass();

    // kommandos.cpp
    void allt();
    void warnung_send();
    void plum_chat();

    // pruefen.cpp
    void pruefen_files();
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
    void main_thread();
    void aktualisieren_thread();
    void nutzer_thread();
    void pruefen_thread();

private slots:
    void vordergrund( bool const status ); // chat.cpp
    void set_pass( std::string newpass ); // kommandos.cpp
};

#endif // CHAT_HPP
