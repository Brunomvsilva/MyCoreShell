#pragma once

#include <string>
#include <vector>
#include "builtins.hpp"    // for CommandContext

// split a raw command line into tokens, honoring quotes and backslashes
std::vector<std::string> tokenizeInput(const std::string& commandLine);

// build a CommandContext (argv, redirections) from the raw input
CommandContext buildCommandContext(const std::string& commandLine);

// search $PATH for an executable with the given name
std::string locateExecutable(const std::string& programName);

// fork/exec a non-builtin with proper I/O redirection
void launchExternalProcess(const CommandContext& ctx);
