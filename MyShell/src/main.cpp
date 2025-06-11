#include "shell.hpp"
#include <readline/history.h>
#include <cstdlib>

int main() {
    // 1) Load existing history (if HISTFILE is set)
    if (const char* historyFile = getenv("HISTFILE")) {
        read_history(historyFile);
    }

    // 2) Ensure history is saved on exit
    atexit([](){
        if (const char* historyFile = getenv("HISTFILE")) {
            write_history(historyFile);
        }
    });

    Shell myShell;
    myShell.run();    // start the interactive shell
    return 0;
}
