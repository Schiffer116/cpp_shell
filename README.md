# Simple shell in cpp

A lightweight interactive shell written in **C++**.
Supports command execution, pipelines, history, and tab completion.

## ✨ Features
- Builtins
    - Exit
    - Echo
    - Type
    - Pwd
    - Cd
- Single/double quoting
- Command execution with output redirection
- Pipelines (`cmd1 | cmd2 | ...`)
- Command history & search (via GNU Readline)
- Tab completion

## ⚙️ Try it out
Make sure you have **GNU Readline** installed (`libreadline-dev`).
```bash
make run
