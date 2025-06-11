#include "builtins.hpp"

class Shell {
public:
    Shell();
    void run();    // was loop()

private:
    static char* completeGenerator(const char* text, int state);
    static char** completeWrapper(const char* text, int start, int end);
    static std::string stripWhitespace(const std::string&);

    void dispatch(const CommandContext& ctx);
    void executePipeline(const std::vector<CommandContext>& stages);
};
