# Copyright (C) 2015 Lukas Bartl
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

# Dies ist die QMake-Projekt-Datei

# Allgemein
TEMPLATE = app
QT      += core gui widgets
CONFIG  += qt

win32 {
   TARGET = binary # mingw unterstützt kein Unicode, deswegen wird nachträglich Hardlink erstellt
} else {
   TARGET = "Handout_Kräuterhexe"
}

CONFIG(release, debug|release) {
    linux {
        DESTDIR = bin
    }
    OBJECTS_DIR = objects
    MOC_DIR = $$OBJECTS_DIR
    RCC_DIR = $$OBJECTS_DIR
    UI_DIR = $$OBJECTS_DIR
}

# Source
HEADERS = functions.hpp \
          datei.hpp \
          datei_mutex.hpp \
          thread.hpp \
          cryptfile.hpp \
          adminpass.hpp \
          hineinschreiben.hpp \
          chat.hpp \
          passwort.hpp \
          warnung.hpp \
          lockfile.hpp \
          nutzer.hpp \
          personalo.hpp \
          entfernen.hpp \
          admin.hpp \
          infoopen.hpp \
          klog.hpp \
          verboten.hpp \
          simpledialog.hpp \
          filesystem.hpp \
          groupopen.hpp

SOURCES = main.cpp \
          definitions.cpp \
          start.cpp \
          threads.cpp \
          aktualisieren.cpp \
          senden.cpp \
          kommandos.cpp \
          privatchats.cpp \
          dialog.cpp \
          pruefen.cpp \
          cryptfile.cpp \
          adminpass.cpp \
          hineinschreiben.cpp \
          chat.cpp \
          passwort.cpp \
          warnung.cpp \
          lockfile.cpp \
          nutzer.cpp \
          personalo.cpp \
          entfernen.cpp \
          admin.cpp \
          infoopen.cpp \
          verboten.cpp \
          simpledialog.cpp \
          groupopen.cpp

FORMS   = chat.ui \
          passwort.ui \
          warnung.ui \
          nutzer.ui \
          lockfile.ui \
          personalo.ui \
          entfernen.ui \
          admin.ui \
          infoopen.ui \
          verboten.ui \
          simpledialog.ui \
          groupopen.ui

# Präprozessor
CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
}

win32 {
    DEFINES += WIN32
}

exists("../config/verwalter") { # Verwalter, chat.cpp
    DEFINES += VERWALTER="\"$$system("./makeconfig.sh verwalter")\""
}

exists("../config/oberadmin") { # Oberadmin, chat.hpp
    DEFINES += OBERADMIN="\"$$system("./makeconfig.sh oberadmin")\""
}

exists("../config/std_admins") { # Standard-Admins, chat.hpp
    DEFINES += STD_ADMINS="\"$$system("./makeconfig.sh std_admins")\""
}

# Kompilieren
QMAKE_CXXFLAGS += -std=c++14 # C++14-Standard
QMAKE_CXXFLAGS_RELEASE -= -g -O2 # Kein Debugging für Release-Versionen
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wextra -Wpedantic -Wdisabled-optimization # Warnungen

linux-clang { # Clang/LLVM
    QMAKE_CXXFLAGS_RELEASE += -O3 # Höchste Optimierung für clang
} else { # GCC
    QMAKE_CXXFLAGS_RELEASE += -Ofast # Optimieren für Schnelligkeit
    QMAKE_CXXFLAGS_DEBUG   += -Og # Optimieren für Debugging
}

CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS_WARN_ON += -Weffc++ -Wsuggest-override -Wsuggest-final-types -Wsuggest-final-methods # Nur für Debugging Versionen
}

# Linken
QMAKE_LFLAGS_RELEASE += -s # strip, unnötige Symbole entfernen

win32 {
    RC_FILE = chat.rc # Windows Icon
    LIBS += -lboost_system-mt -lboost_filesystem-mt
} else {
    LIBS += -lboost_system -lboost_filesystem
}
