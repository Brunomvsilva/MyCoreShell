// src/utils.cpp

#include "utils.hpp"
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <fcntl.h>
#include <vector>
#include <string>

using namespace std;

// Tokenize input respecting single- and double-quotes and backslashes
vector<string> tokenizeInput(const string& input) {
    vector<string> tokens;
    string currentToken;
    bool inSingleQuotes = false;
    bool inDoubleQuotes = false;
    bool escapeNextChar = false;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];

        if (escapeNextChar) {
            if (inDoubleQuotes) {
                // In double quotes, only certain escapes are removed
                if (c == '\\' || c == '"' || c == '$' || c == '\n') {
                    currentToken += c;
                } else {
                    // preserve the backslash for other chars
                    currentToken += '\\';
                    currentToken += c;
                }
            } else if (!inSingleQuotes) {
                // Outside quotes, backslash escapes any char
                currentToken += c;
            } else {
                // In single quotes, backslash is literal
                currentToken += '\\';
                currentToken += c;
            }
            escapeNextChar = false;
            continue;
        }

        if (c == '\\') {
            if (inSingleQuotes) {
                // literal backslash in single quotes
                currentToken += '\\';
            } else {
                escapeNextChar = true;
            }
            continue;
        }

        if (c == '\'' && !inDoubleQuotes) {
            inSingleQuotes = !inSingleQuotes;
            continue;
        }

        if (c == '"' && !inSingleQuotes) {
            inDoubleQuotes = !inDoubleQuotes;
            continue;
        }

        if (isspace(static_cast<unsigned char>(c)) && !inSingleQuotes && !inDoubleQuotes) {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else {
            currentToken += c;
        }
    }

    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }

    return tokens;
}

// Build a CommandContext from a raw command line (handles redirections)
CommandContext buildCommandContext(const string& commandLine) {
    auto rawTokens = tokenizeInput(commandLine);
    CommandContext ctx;
    for (size_t i = 0; i < rawTokens.size(); ++i) {
        const auto& t = rawTokens[i];
        if ((t == ">>" || t == "1>>") && i + 1 < rawTokens.size()) {
            ctx.outputRedirectPath = rawTokens[++i];
            ctx.outputAppend = true;
        } else if ((t == ">" || t == "1>") && i + 1 < rawTokens.size()) {
            ctx.outputRedirectPath = rawTokens[++i];
            ctx.outputAppend = false;
        } else if (t == "2>>" && i + 1 < rawTokens.size()) {
            ctx.errorRedirectPath = rawTokens[++i];
            ctx.errorAppend = true;
        } else if (t == "2>" && i + 1 < rawTokens.size()) {
            ctx.errorRedirectPath = rawTokens[++i];
            ctx.errorAppend = false;
        } else {
            ctx.argv.push_back(t);
        }
    }
    ctx.argc = static_cast<int>(ctx.argv.size());
    return ctx;
}

// Search $PATH for an executable
string locateExecutable(const string& programName) {
    if (const char* pathEnv = getenv("PATH")) {
        istringstream paths(pathEnv);
        string dir;
        while (getline(paths, dir, ':')) {
            string candidate = dir + "/" + programName;
            if (access(candidate.c_str(), X_OK) == 0)
                return candidate;
        }
    }
    return {};
}

// Helper to build argv for execvp
struct ArgvArray {
    vector<char*> ptrs;
    ArgvArray(const vector<string>& args) {
        for (const auto& s : args)
            ptrs.push_back(strdup(s.c_str()));
        ptrs.push_back(nullptr);
    }
    ~ArgvArray() {
        for (auto p : ptrs) free(p);
    }
    char** data() { return ptrs.data(); }
};

// Fork and exec an external command with I/O redirection
void launchExternalProcess(const CommandContext& ctx) {
    ArgvArray argv(ctx.argv);
    pid_t pid = fork();
    if (pid == 0) {
        if (!ctx.outputRedirectPath.empty()) {
            int flags = O_WRONLY | O_CREAT |
                        (ctx.outputAppend ? O_APPEND : O_TRUNC);
            int fd = open(ctx.outputRedirectPath.c_str(), flags, 0644);
            if (fd < 0) { perror("open"); exit(1); }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if (!ctx.errorRedirectPath.empty()) {
            int flags = O_WRONLY | O_CREAT |
                        (ctx.errorAppend ? O_APPEND : O_TRUNC);
            int fd = open(ctx.errorRedirectPath.c_str(), flags, 0644);
            if (fd < 0) { perror("open"); exit(1); }
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
        execvp(argv.data()[0], argv.data());
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, nullptr, 0);
    } else {
        perror("fork");
    }
}
