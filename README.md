# Klassenchat

[Quellcode](https://github.com/hanswurst862/klassenchat)

[Dokumentation](https://hanswurst862.github.io/klassenchat)

[Releases](https://github.com/hanswurst862/klassenchat/releases)

## Kompilieren
Herunterladen des Quellcodes:

    git clone --recursive https://github.com/hanswurst862/klassenchat.git
    cd klassenchat
    
Aktualisieren des Quellcodes:
    
    ./update
    
Zum Einrichten (inklusive installieren des Cross-Compilers MXE und kompilieren der statischen Qt-Bibliotheken):

    ./configure

Um für Windows zu kompilieren:

    ./compile win32

Um für Unix zu kompilieren:

    ./compile unix
    
Um für das native System zu kompilieren:

    ./compile native
    
Um für alles gleichzeitig zu kompilieren:
    
    ./compile all
