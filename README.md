# MiniShell – Unix-like Shell in C

## Overview
MiniShell is a custom command-line shell developed in C on Linux. It supports execution of external commands, built-in commands, pipelines, and job control.

The project demonstrates practical concepts of Linux system programming, process management, signal handling, and inter-process communication.

## Features
- Execute external commands using `fork()` and `execvp()`
- Built-in commands: `cd`, `pwd`, `echo`, `exit`
- Job control commands: `jobs`, `fg`, and `bg`
- Foreground and background process management
- Pipeline execution using `pipe()` and `dup2()`
- Signal handling for `SIGINT` and `SIGTSTP`
- Custom shell prompt using `PS1`
- Job tracking using a linked list

## Technologies
- C Programming
- Linux / Ubuntu
- POSIX System Calls
- Process Management
- Signals
- Inter-Process Communication

## Project Structure
```text
MiniShell/
├── main.c
├── main.h
├── command.txt
├── makefile
└── README.md
```

## Compilation
```bash
make
```

## Execution
```bash
./mini.exe
```

## Sample Commands
```bash
MINISHELL$ pwd
MINISHELL$ ls
MINISHELL$ echo Hello
MINISHELL$ sleep 100
MINISHELL$ jobs
MINISHELL$ bg
MINISHELL$ fg
MINISHELL$ exit
```

## Learning Outcomes
- Understanding process creation and execution
- Implementing job control and signal handling
- Working with pipes and process communication
- Developing a deeper understanding of Linux shell internals

## Author
Shree Vathsa

## License
For educational purposes.
