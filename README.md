# My C++ Shell

A minimal yet powerful Unix shell implemented in **modern C++**, featuring command execution, piping, history, tab completion, redirection, and built-in commands. Clean architecture, idiomatic C++17, and full readline support — all in under ~1k lines of well-documented code.

---

## 🚀 Features

- ✅ **Built-in commands**:  
  `cd`, `pwd`, `exit`, `echo`, `type`, `history`
- ✅ **External commands**:  
  Uses `$PATH` to locate and run binaries
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
- ✅ **Auto-completion**:  
  Tab-complete built-ins and executables via `readline`

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

