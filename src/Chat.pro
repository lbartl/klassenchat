# Copyright (C) 2015,2016 Lukas Bartl
# Diese Datei ist Teil des Klassenchats.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Dies ist die QMake-Projekt-Datei für den Chat

# Allgemein
TEMPLATE = app
QT      += core gui widgets
CONFIG  += qt

win32 {
    TARGET = "binary1" # nachträglicher Hardlink um .exe zu entfernen
} else {
    TARGET = "hi.jpg"
}

CONFIG(release, debug|release) {
    linux {
        DESTDIR = "bin/S.75_3"
    }
    OBJECTS_DIR = objects
    MOC_DIR = $$OBJECTS_DIR
    RCC_DIR = $$OBJECTS_DIR
    UI_DIR = $$OBJECTS_DIR
} else {
    DESTDIR = "S.75_3"
}

# Source
HEADERS = datei.hpp \
          datei_mutex.hpp \
          thread.hpp \
          cryptfile.hpp \
          adminpass.hpp \
          chat.hpp \
          passwort.hpp \
          warnung.hpp \
          nutzer.hpp \
          personalo.hpp \
          entfernen.hpp \
          infoopen.hpp \
          klog.hpp \
          simpledialog.hpp \
          filesystem.hpp \
          nutzer_anz.hpp \
          global.hpp \
          forkbomb.hpp \
          pc_nutzername.hpp \
          chatverwaltung.hpp \
          ueber.hpp

SOURCES = main.cpp \
          definitions.cpp \
          start.cpp \
          threads.cpp \
          aktualisieren.cpp \
          senden.cpp \
          kommandos.cpp \
          dialog.cpp \
          pruefen.cpp \
          cryptfile.cpp \
          adminpass.cpp \
          chat.cpp \
          passwort.cpp \
          warnung.cpp \
          personalo.cpp \
          entfernen.cpp \
          infoopen.cpp \
          verboten.cpp \
          simpledialog.cpp \
          nutzer_anz.cpp \
          nutzer.cpp \
          pc_nutzername.cpp \
          chatverwaltung.cpp

FORMS   = chat.ui \
          passwort.ui \
          warnung.ui \
          personalo.ui \
          entfernen.ui \
          infoopen.ui \
          verboten.ui \
          simpledialog.ui \
          admin_anz.ui \
          nutzer_anz.ui \
          forkbomb.ui

# Präprozessor
CONFIG(debug, debug|release) {
    DEFINES += DEBUG _GLIBCXX_DEBUG
} else {
    DEFINES += _FORTIFY_SOURCE=2 QT_NO_DEBUG_OUTPUT
}

win32 {
    DEFINES += WIN32
}

exists("../config/verwalter") { # Verwalter, ueber.hpp
    DEFINES += VERWALTER="\"$$system("./makeconfig.sh verwalter")\""
}

exists("../config/oberadmin") { # Oberadmin, chat.hpp
    DEFINES += OBERADMIN="\"$$system("./makeconfig.sh oberadmin")\""
}

exists("../config/std_admins") { # Standard-Admins, chat.hpp
    DEFINES += STD_ADMINS="\"$$system("./makeconfig.sh std_admins")\""
}

# Kompilieren
CONFIG += c++14
QMAKE_CXXFLAGS_RELEASE = -mtune=generic -fstack-protector-strong -flto -ffast-math -O3 # Optimieren für Schnelligkeit
QMAKE_CXXFLAGS_DEBUG = -mtune=native -ggdb3 -Og # Optimieren für Debugging
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wextra -Wpedantic -Wdisabled-optimization -Werror=format-security # Warnungen

CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS_WARN_ON += -Weffc++ -Wsuggest-override -Wsuggest-final-types -Wsuggest-final-methods # Nur bei Debug-Versionen, sonst gibt es Warnungen von Bibliotheken
}

linux {
    QMAKE_CXXFLAGS_RELEASE += -fPIE
}

native {
    QMAKE_CXXFLAGS_RELEASE += -march=native
}

# Linken
linux {
    QMAKE_LFLAGS_RELEASE = -Wl,-O1,-z,relro,-z,now -pie -s # Optimierung, RELRO und Strip
}

QMAKE_LFLAGS_RELEASE += -flto # Link Time Optimization

CONFIG(debug, debug|release) {
    LIBS += -lboost_system -lboost_filesystem
} else:native {
    LIBS += -lboost_system -lboost_filesystem
} else:win32 {
    LIBS += -lboost_system-mt -lboost_filesystem-mt -lboost_thread_win32-mt
} else {
    LIBS += -static-libstdc++ -Wl,-Bstatic -lboost_system -lboost_filesystem -Wl,-Bdynamic
}
