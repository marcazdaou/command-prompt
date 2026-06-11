# bsh — Custom Unix Shell

A Unix shell interpreter written in **C** that implements core shell functionality from scratch — command parsing, process management, environment control, and command history.

## Features

| Feature | Details |
|---|---|
| **Command execution** | Searches `PATH` directories and runs executables via `fork` / `execv` |
| **Command parsing** | Tokenizes input into up to 16 arguments per command |
| **Directory navigation** | `cd [path]` — defaults to `$HOME`, updates `PWD` environment variable |
| **Environment variables** | `environ` (list), `setenv VAR VALUE` (set/update), `unsetenv VAR` (remove) |
| **Command history** | Stores last 500 commands; `history` prints numbered list |
| **Input limits** | Up to 1024-byte command lines, up to 64 environment variables |

## Built-in Commands

```
cd [path]            Change directory (defaults to $HOME)
environ              List all environment variables
setenv VAR VALUE     Set or update an environment variable
unsetenv VAR         Remove an environment variable
history              Print command history (last 500 commands)
exit                 Exit the shell
```

## Implementation Details

### Process Management
Each external command spawns a child process via `fork()`. The child resolves the executable by iterating through `PATH` directories, then calls `execv()`. The parent waits for the child to complete.

```c
pid_t pid = fork();
if (pid == 0) {
    execv(full_path, args);   // child: replace with target program
} else {
    waitpid(pid, &status, 0); // parent: wait for child
}
```

### PATH Resolution
The shell tokenizes the `PATH` environment variable on `:` and checks each directory for an executable matching the command name using `access(path, X_OK)`.

### Command History
A circular buffer of 500 `char*` entries. When the buffer is full, the oldest entry is freed and all entries shift left by one.

### Environment Management
Environment variables are maintained in a separate `environcopy[]` array (independent of `envp`) so `setenv`/`unsetenv` changes persist across commands within the session.

## Build & Run

```bash
gcc -o bsh bsh.c
./bsh
```

```
bsh> ls -la
bsh> cd projects
bsh> setenv EDITOR vim
bsh> history
bsh> exit
```

## Technologies

- **C**
- POSIX: `fork`, `execv`, `waitpid`, `chdir`, `getcwd`, `access`
- Manual memory management (`malloc`, `strdup`, `free`)