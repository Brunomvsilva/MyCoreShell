#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
using namespace std;

// Parsed command + redirection info
struct CommandContext {
    vector<string> argv;
    int argc = 0;
    string outputRedirectPath;
    bool outputAppend = false;
    string errorRedirectPath;
    bool errorAppend = false;
};

// Base for shell builtins
struct ShellCommand {
    virtual ~ShellCommand() = default;
    virtual void run(const CommandContext& ctx) = 0;
};

extern map<string, unique_ptr<ShellCommand>> builtins;
void registerBuiltins();
