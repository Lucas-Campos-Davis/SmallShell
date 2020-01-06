# SmallShell

This is a small shell written in C for Linux

## Overview

This shell has three built-in commands: exit, cd, and status.

exit exits the shell

cd changes directories. If no argument is given it will change directories to the location specified in the HOME environment variable. Otherwise it will change directories to the location given as an argument.

status prints ount either the exit status or the terminating signal of the last foreground process

The shell handles other commands as well using fork and exec.

The shell can run commands in the background, to do so add a "&" to the end of the command. 
The shell will print the process id of the background process and when i t finishes, prints the process id and exit status.

The shell expands "$$" to the process id of the shell to facitlitate easier testing.

CTRL-C from the keyboard will only kill the current foreground process, not the shell.

CTRL-Z from the keyboard will turn foreground-only mode on or off. Foreground-only mode treats background commands as foreground commands.

##Installing and Compiling

To install: clone this repo.

To compile:
```bash
gcc -o smallsh driver.c
```

To Run:
```bash
./smallsh
```
