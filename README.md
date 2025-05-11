# System Programming by Linux

This repository contains various C++ implementations of Unix-like commands and shell programs developed as part of a system programming course using Linux. These programs demonstrate low-level system call usage, process control, and custom shell functionality.

## 📁 Repository Structure

System_programming_by_Linux/
├── cp.cpp # Custom implementation of the 'cp' command
├── echo.cpp # Custom implementation of the 'echo' command
├── femtoshell.cpp # Lightweight shell implementation (femtoshell)
├── microshell.cpp # Medium-level shell with variable support and redirection
├── mv.cpp # Custom implementation of the 'mv' command
├── nanoshell.cpp # Simpler shell handling basic execution (nanoshell)
├── picoshell.cpp # Very minimal shell (picoshell)
├── pwd.cpp # Custom implementation of the 'pwd' command
└── README.md # Project documentation

## 🧠 Features

- ✅ Basic command utilities: `cp`, `mv`, `pwd`, `echo`
- 🐚 Multiple custom shell programs:
  - `picoshell`: Executes simple commands.
  - `nanoshell`: Adds some parsing and execution features.
  - `microshell`: Supports variable assignments, redirection (`<`, `>`, `2>`), and built-in commands like `export`, `cd`, `echo`, `pwd`.
  - `femtoshell`: A tiny shell designed for lightweight interactions.
- 🔁 Uses `fork()`, `execvp()`, `dup()`, `dup2()`, `open()`, `close()` system calls.
- 🧪 Built-in support for command-line parsing and environment manipulation.

## 🛠️ How to Compile & Run

Compile any file using `g++`:

bash
```g++ filename.cpp -o outputname
./outputname
```
## 🚀 Sample Commands (for microshell/femtoshell)

These custom shells support a variety of commands that mimic standard Unix shell behavior.

### 🔧 Built-in Commands

- `pwd` — Show the current working directory  
- `cd /path/to/dir` — Change the current working directory  
- `echo Hello World` — Print text to the terminal  
- `exit` — Exit the shell

### 💡 Variable Handling

- `MYVAR=value` — Define a local shell variable  
- `echo $MYVAR` — Access and print the value of a shell variable  
- `export MYVAR` — Export a local variable to the environment

### 📤 Input/Output Redirection

- `command > output.txt` — Redirect standard output to a file  
- `command 2> error.txt` — Redirect standard error to a file  
- `command < input.txt` — Redirect standard input from a file  
- `command > out.txt 2> err.txt` — Redirect both stdout and stderr

### 🧪 Examples

```sh
# Print working directory
pwd

# Change to /tmp directory
cd /tmp

# Set and print a variable
MYNAME=Ziad
echo $MYNAME

# Export a variable and use it in a child process
export MYNAME
./some_script.sh  # can now access $MYNAME

# Redirect output to file
echo Hello > hello.txt

# Run a command with error output redirected
ls non_existing_file 2> error.log

# Exit the shell
exit
