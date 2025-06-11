# My C++ Shell

A minimalUnix shell implemented in **modern C++**, featuring command execution, piping, history, arrow history navigation, tab completion, output redirection, and some built-in commands.

---

## 🚀 Features

- ✅ **Built-in commands**:  
  `cd`, `pwd`, `exit`, `echo`, `type`, `history`
- ✅ **External commands (executables)**:  
  Supports any binary found in your system's `$PATH`, such as:  
  `ls`, `cat`, `grep`, `echo`, `mkdir`, `touch`, `rm`, `pwd`, `tail`, etc..
- ✅ **Pipes**:  
  Full support for `|` operator (e.g. `ls | grep txt`)
- ✅ **Redirection**:  
  `>`, `>>`, `2>`, `2>>` for stdout/stderr
- ✅ **Quote-aware parser**:  
  Handles `'single'`, `"double"`, and escaped `\` characters
- ✅ **Command history**:
  - `HISTFILE` support with persistent history
  - `history -r <file>` — read from file
  - `history -w <file>` — write to file
  - `history -a <file>` — append session history
- ✅ **Arrow Navigation**:
  - Use Arrows to execute the last used commands  
- ✅ **Auto-completion**:  
  Tab-complete built-ins and executables via `readline`

---

## 📚 How to Build

Ensure dependencies are installed and build executable:

```sh
sudo apt install cmake libreadline-dev
chmod +x buildProject.sh
./buildProject.sh
./build/shell
```
---

## 📂 Example Session

```sh
$ echo Hello World
Hello World

$ cd /tmp
$ pwd
/tmp

$ ls | grep log > logs.txt
$ history
   1  echo Hello World
   2  cd /tmp
   3  pwd
   4  ls | grep log > logs.txt
   5  history
```
## 🛠️ Architecture Overview

| File(s)                     | Responsibility                                        |
|----------------------------|--------------------------------------------------------|
| `main.cpp`                 | History management and program entry point             |
| `shell.cpp` / `shell.hpp`  | REPL loop, pipeline execution, and tab-completion      |
| `utils.cpp` / `utils.hpp`  | Tokenizer, `$PATH` search, external command helpers    |
| `builtins.cpp` / `builtins.hpp` | Built-in commands (`cd`, `pwd`, `echo`, etc.) implementation |

