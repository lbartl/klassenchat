# Klassenchat

[Quellcode](https://github.com/hanswurst862/klassenchat)

[Dokumentation](http://hanswurst862.github.io/klassenchat)

## Kompilieren
Herunterladen des Quellcodes:

    git clone https://github.com/hanswurst862/klassenchat
    cd klassenchat
    
Zum Einrichten:

    ./configure

Um für Windows zu kompilieren:

    ./compile win32

Dabei wird zuerst der Cross-Compiler heruntergeladen und kompiliert (in ~/bin/mxe). Anschließend wird der Chat kompiliert

Um für Unix zu kompilieren:

    ./compile unix
    
Um für beides gleichzeitig zu kompilieren:
    
    ./compile all
