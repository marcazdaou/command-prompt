# bsh: a basic Unix shell in C

A small Unix shell written in C for a systems programming course. It reads commands in a loop, runs built-in
commands itself, and starts other programs by searching the `PATH` and using `fork` / `execv` / `waitpid`.

## Built-in commands

| Command | What it does |
|---|---|
| `cd [dir]` | Change directory (to `$HOME` if no argument) and update `PWD` |
| `env` | Print the shell's environment variables |
| `setenv VAR VALUE` | Add or update a variable |
| `unsetenv VAR` | Remove a variable |
| `history` | Print the numbered list of previous commands (up to 500) |
| `exit` | Quit the shell |

Any other command is looked up in each `PATH` directory with `access(path, X_OK)`; the first match is run in a
child process with `fork` + `execv`, and the shell waits for it with `waitpid`.

## Implementation notes (`bsh.c`)

- **Parsing:** each line is split on spaces with `strsep` into up to 16 arguments (lines up to 1,024 bytes).
- **Environment:** the shell keeps its own copy of the environment (up to 64 variables), so `setenv`,
  `unsetenv` and `cd` change that copy.
- **History:** an array of the last 500 commands; when it is full, the oldest entry is freed and the rest shift down.
- **Debug mode:** `./bsh -d` prints each parsed argument and which code path runs.
- **Current limitation:** external programs are started with their name only; their arguments are parsed but
  not yet passed to `execv`, so `ls -la` runs as `ls`.

## Build & run

```bash
gcc -o bsh bsh.c
./bsh          # or ./bsh -d for debug output
```

```
bsh> cd /tmp
bsh> setenv EDITOR vim
bsh> env
bsh> ls
bsh> history
bsh> exit
```

## Concepts

Processes (`fork`, `execv`, `waitpid`) · `PATH` resolution · environment variables · string parsing in C · manual memory management
