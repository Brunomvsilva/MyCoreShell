// shell.cpp

#include "shell.hpp"
#include "utils.hpp"

#include <readline/readline.h>
#include <readline/history.h>
#include <sstream>
#include <iostream>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <algorithm>
#include <unordered_set>
#include <array>
#include <cstring>
#include <cstdlib>

using namespace std;

Shell::Shell() {
    cout << unitbuf;
    cerr << unitbuf;
    registerBuiltins();
    // Hook up readline to our completion routines
    rl_attempted_completion_function = Shell::completeWrapper;
}

void Shell::run() {
    while (true) {
        char* lineRead = readline("$ ");
        if (!lineRead) {
            cout << endl;
            break;
        }
        string line(lineRead);
        free(lineRead);

        if (line.empty()) continue;
        add_history(line.c_str());

        // Split on '|' for pipelines
        istringstream pipelineStream(line);
        vector<string> segments;
        string segment;
        while (getline(pipelineStream, segment, '|')) {
            string trimmed = stripWhitespace(segment);
            if (!trimmed.empty())
                segments.push_back(trimmed);
        }

        if (segments.size() > 1) {
            vector<CommandContext> contexts;
            for (auto& seg : segments)
                contexts.push_back(buildCommandContext(seg));
            executePipeline(contexts);
        } else {
            CommandContext ctx = buildCommandContext(line);
            if (!ctx.argv.empty())
                dispatch(ctx);
        }
    }
}

char** Shell::completeWrapper(const char* text, int start, int end) {
    rl_attempted_completion_over = 1;
    if (start == 0)
        return rl_completion_matches(text, Shell::completeGenerator);
    return nullptr;
}

char* Shell::completeGenerator(const char* text, int state) {
    static vector<string> matches;
    if (state == 0) {
        matches.clear();
        size_t prefixLen = strlen(text);
        unordered_set<string> seen;

        // Built-in commands
        for (auto& p : builtins) {
            if (p.first.compare(0, prefixLen, text) == 0) {
                matches.push_back(p.first);
                seen.insert(p.first);
            }
        }

        // Executables in PATH
        if (const char* pathEnv = getenv("PATH")) {
            istringstream pathStream(pathEnv);
            string dir;
            while (getline(pathStream, dir, ':')) {
                DIR* dp = opendir(dir.c_str());
                if (!dp) continue;
                struct dirent* de;
                while ((de = readdir(dp)) != nullptr) {
                    string name(de->d_name);
                    if (name.size() < prefixLen ||
                        name.compare(0, prefixLen, text) != 0)
                        continue;
                    if (seen.count(name))
                        continue;
                    string full = dir + "/" + name;
                    if (access(full.c_str(), X_OK) == 0) {
                        matches.push_back(name);
                        seen.insert(name);
                    }
                }
                closedir(dp);
            }
        }
        sort(matches.begin(), matches.end());
    }
    if (state < static_cast<int>(matches.size()))
        return strdup(matches[state].c_str());
    return nullptr;
}

string Shell::stripWhitespace(const string& s) {
    size_t first = s.find_first_not_of(" \t");
    if (first == string::npos) return "";
    size_t last = s.find_last_not_of(" \t");
    return s.substr(first, last - first + 1);
}

void Shell::dispatch(const CommandContext& ctx) {
    int savedOut = -1, savedErr = -1;

    // Setup stdout redirection
    if (!ctx.outputRedirectPath.empty()) {
        savedOut = dup(STDOUT_FILENO);
        int flags = O_WRONLY | O_CREAT |
                    (ctx.outputAppend ? O_APPEND : O_TRUNC);
        int fd = open(ctx.outputRedirectPath.c_str(), flags, 0644);
        if (fd < 0) {
            perror("open stdout");
            if (savedOut >= 0)
                close(savedOut);
            return;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    // Setup stderr redirection
    if (!ctx.errorRedirectPath.empty()) {
        savedErr = dup(STDERR_FILENO);
        int flags = O_WRONLY | O_CREAT |
                    (ctx.errorAppend ? O_APPEND : O_TRUNC);
        int fd = open(ctx.errorRedirectPath.c_str(), flags, 0644);
        if (fd < 0) {
            perror("open stderr");
            if (savedErr >= 0)
                close(savedErr);
            return;
        }
        dup2(fd, STDERR_FILENO);
        close(fd);
    }

    // Built-in?
    auto it = builtins.find(ctx.argv[0]);
    if (it != builtins.end()) {
        it->second->run(ctx);
    } else {
        // External
        if (locateExecutable(ctx.argv[0]).empty()) {
            cerr << ctx.argv[0] << ": command not found\n";
        } else {
            launchExternalProcess(ctx);
        }
    }

    // Restore stdout/stderr
    if (savedOut >= 0) {
        dup2(savedOut, STDOUT_FILENO);
        close(savedOut);
    }
    if (savedErr >= 0) {
        dup2(savedErr, STDERR_FILENO);
        close(savedErr);
    }
}

void Shell::executePipeline(const vector<CommandContext>& stages) {
    size_t n = stages.size();
    vector<array<int,2>> pipes(n > 1 ? n-1 : 0);

    // Create pipes
    for (size_t i = 0; i + 1 < n; ++i) {
        if (pipe(pipes[i].data()) < 0) {
            perror("pipe");
            return;
        }
    }

    // Fork each stage
    vector<pid_t> children;
    for (size_t i = 0; i < n; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return;
        }
        if (pid == 0) {
            // Child: wire up pipes
            if (i > 0)
                dup2(pipes[i-1][0], STDIN_FILENO);
            if (i + 1 < n)
                dup2(pipes[i][1], STDOUT_FILENO);
            for (auto& pr : pipes) {
                close(pr[0]);
                close(pr[1]);
            }
            dispatch(stages[i]);
            exit(0);
        }
        children.push_back(pid);
    }

    // Parent: close pipes and wait
    for (auto& pr : pipes) {
        close(pr[0]);
        close(pr[1]);
    }
    for (pid_t c : children)
        waitpid(c, nullptr, 0);
}
