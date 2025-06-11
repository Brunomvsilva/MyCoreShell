#include "shell.hpp"
#include <readline/history.h>
#include <cstdlib>

void saveHistory() {
    if (const char* historyFile = getenv("HISTFILE")) {
        write_history(historyFile);
    }
}

int main() {
    if (const char* historyFile = getenv("HISTFILE")) {
        read_history(historyFile);
    }

    atexit(saveHistory);

    Shell myShell;
    myShell.run();
    return 0;
}