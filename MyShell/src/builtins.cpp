// src/builtins.cpp

#include "builtins.hpp"
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <limits.h>
#include <iomanip>
#include <readline/history.h>

using namespace std;

// define the global builtins map exactly once
map<string, unique_ptr<ShellCommand>> builtins;

// echo
struct EchoCommand : ShellCommand {
    void run(const CommandContext& ctx) override {
        for (size_t i = 1; i < ctx.argv.size(); ++i) {
            cout << ctx.argv[i] << (i+1<ctx.argv.size()?" ":"");
        }
        cout << "\n";
    }
};

// exit
struct ExitCommand : ShellCommand {
    void run(const CommandContext& ctx) override {
        int code = 0;
        if (ctx.argv.size()>1) {
            try { code = stoi(ctx.argv[1]); }
            catch(...) { cerr<<"exit: invalid code\n"; return; }
        }
        exit(code);
    }
};

// type
struct TypeCommand : ShellCommand {
    void run(const CommandContext& ctx) override {
        if (ctx.argv.size()<2) { cout<<"type: missing operand\n"; return; }
        string name = ctx.argv[1];
        if (builtins.count(name)) {
            cout<<name<<" is a shell builtin\n";
        }
        else if (auto path = locateExecutable(name); !path.empty()) {
            cout<<name<<" is "<<path<<"\n";
        }
        else {
            cout<<name<<": not found\n";
        }
    }
};

// pwd
struct PwdCommand : ShellCommand {
    void run(const CommandContext&) override {
        char buf[PATH_MAX];
        if (getcwd(buf,sizeof(buf))) cout<<buf<<"\n";
        else perror("pwd");
    }
};

// cd
struct CdCommand : ShellCommand {
    void run(const CommandContext& ctx) override {
        string tgt;
        if (ctx.argv.size()<2) {
            if (auto h = getenv("HOME"); h && *h) {
                tgt = h;
            } else {
                cerr<<"cd: HOME not set\n"; return;
            }
        } else {
            tgt = ctx.argv[1];
        }
        if (tgt=="~"||tgt.rfind("~/",0)==0) {
            if (auto h=getenv("HOME")) tgt = string(h)+tgt.substr(1);
            else { cerr<<"cd: HOME not set\n"; return; }
        }
        if (chdir(tgt.c_str())!=0) cerr<<"cd: "<<tgt<<": "<<strerror(errno)<<"\n";
    }
};

// history
struct HistoryCommand : ShellCommand {
    void run(const CommandContext& ctx) override {
        // -r, -w, -a as before...
        if (ctx.argv.size()==3 && ctx.argv[1]=="-r") {
            ifstream f(ctx.argv[2]);
            if (!f) { cerr<<"history: cannot open "<<ctx.argv[2]<<"\n"; return; }
            string line; while(getline(f,line)) if(!line.empty()) add_history(line.c_str());
            return;
        }
        if (ctx.argv.size()==3 && ctx.argv[1]=="-w") {
            ofstream f(ctx.argv[2]);
            if (!f) { cerr<<"history: cannot open "<<ctx.argv[2]<<" for writing\n"; return; }
            if (auto h=history_list()) {
                int cnt=0; while(h[cnt]) ++cnt;
                for(int i=0;i<cnt;++i) f<<h[i]->line<<"\n";
            }
            return;
        }
        if (ctx.argv.size()==3 && ctx.argv[1]=="-a") {
            static int last=0;
            ofstream f(ctx.argv[2], ios::app);
            if (!f) { cerr<<"history: cannot open "<<ctx.argv[2]<<" for appending\n"; return; }
            if (auto h=history_list()) {
                int total=0; while(h[total]) ++total;
                for(int i=last;i<total;++i) f<<h[i]->line<<"\n";
                last=total;
            }
            return;
        }
        if (auto h=history_list()) {
            int total=0; while(h[total]) ++total;
            int limit=total;
            if (ctx.argv.size()==2) {
                try { int n=stoi(ctx.argv[1]); if(n>0&&n<limit) limit=n; }
                catch(...){}
            }
            int base=history_base;
            for(int i=total-limit;i<total;++i)
                cout<<setw(5)<<(base+i)<<"  "<<h[i]->line<<"\n";
        }
    }
};

void registerBuiltins() {
    builtins["echo"]    = make_unique<EchoCommand>();
    builtins["exit"]    = make_unique<ExitCommand>();
    builtins["type"]    = make_unique<TypeCommand>();
    builtins["pwd"]     = make_unique<PwdCommand>();
    builtins["cd"]      = make_unique<CdCommand>();
    builtins["history"] = make_unique<HistoryCommand>();
}
