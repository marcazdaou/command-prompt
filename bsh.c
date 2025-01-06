// Marc Daou

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAXARG 16       					//accept up to 16 command-line arguments

#define MAXENV 64						//allow up to 64 environment variables

#define HISTSIZE 500						//keep the last 500 commands in history

#define MAXLINE 1024						//accept up to 1024 bytes in one command


char *environcopy[MAXENV];     					// array to store env variable 

int environcount = 0;						// count of env variables


char *history[HISTSIZE];					// Array store command history

int histcount = 0;						// Track count for the commands history


static char **parseCmd(char cmdLine[]) {			// Function to parse command line input into tokens
    char **cmdArg, *ptr;
    int i;

      cmdArg = (char **) malloc(sizeof(char *) * (MAXARG + 1)); //(MAXARG + 1) because the list must be terminated by a NULL ptr

  if (cmdArg == NULL) {
    perror("parseCmd: cmdArg is NULL");
    exit(1);
  }
  for (i = 0; i <= MAXARG; i++) //note the equality
    cmdArg[i] = NULL;
  i = 0;
  ptr = strsep(&cmdLine, " ");
  while (ptr != NULL) {
    								// (strlen(ptr) + 1)
    cmdArg[i] = (char *) malloc(sizeof(char) * (strlen(ptr) + 1));
    if (cmdArg[i] == NULL) {
      perror("parseCmd: cmdArg[i] is NULL");
      exit(1);
    }
    strcpy(cmdArg[ i++ ], ptr);
    if (i == MAXARG)
      break;
    ptr = strsep(&cmdLine, " ");
  }
  return(cmdArg);
}


void copy_environ(char *envp[]) {				// copy environment variables	
    int i = 0;    						// i is 0.
    while (envp[i] != NULL && i < MAXENV) {			// if it is not null and less that MAXENV
        environcopy[i] = strdup(envp[i]);
        if (environcopy[i] == NULL) {
            perror("Failed Hahaha");	// Failed memory
            exit(1);
        }
        i++;							// increment i
    }
    environcount = i;
}

void command_environ() {						// Function that lists environment variable.
if (environcount == 0) {					// if it is 0
        printf("NOP.\n");		//print
    }
    for (int i = 0; i < environcount; i++) {
        printf("%s\n", environcopy[i]);				// print
    }
}


void command_setenv(char *var, char *value) {			// Function to set or update an environment variable
    int i;
    char new_entry[MAXLINE];

    snprintf(new_entry, MAXLINE, "%s=%s", var, value);


    for (i = 0; i < environcount; i++) {				// Check if variable exists in environcopy
        if (strncmp(environcopy[i], var, strlen(var)) == 0 && environcopy[i][strlen(var)] == '=') {
            free(environcopy[i]);
            environcopy[i] = strdup(new_entry);
            return;
        }
    }

    
    if (environcount < MAXENV) {					// Add new variable if it doesn't exist
        environcopy[environcount++] = strdup(new_entry);
    } else {
        printf("SLOW DOWN LIMIT REACHED.\n");
    }
}

void command_unsetenv(char *var) {					// Function to remove an environment variable
    int i, j;
    for (i = 0; i < environcount; i++) {
        if (strncmp(environcopy[i], var, strlen(var)) == 0 && environcopy[i][strlen(var)] == '=') {
            free(environcopy[i]);
            for (j = i; j < environcount - 1; j++) {
                environcopy[j] = environcopy[j + 1];
            }
            environcopy[--environcount] = NULL;
            return;
        }
    }
    printf("Environment variable %s not found.\n", var);
}

void command_changedirectory(char *path) {					// Function to change directories
    char cwd[MAXLINE];
    if (path == NULL) {
        path = getenv("HOME"); 					// default to home directory
    }
    if (chdir(path) == 0) {
        getcwd(cwd, sizeof(cwd));
        command_setenv("PWD", cwd); 				// update PWD environment variable
    } else {
        perror("CD FAIL HAHA");
    }
}

void add_to_history(char *cmdLine) {				// Function to add a command to history
    if (histcount == HISTSIZE) {
        free(history[0]);
        for (int i = 1; i < HISTSIZE; i++) {
            history[i - 1] = history[i];
        }
        histcount--;
    }
    history[histcount++] = strdup(cmdLine);
}

void command_history() {						// Function to display command history
    for (int i = 0; i < histcount; i++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}


int search_execute (char *cmd) {				// Function to search and execute commands based of PATH
    char *path_env = getenv("PATH");  				// PATH environment variable
    char *path_copy = strdup(path_env);  			// Copy PATH
    char *dir;
    char full_path[MAXLINE];

   
    dir = strsep(&path_copy, ":");				// Iterate through the directories in PATH
    while (dir != NULL) {
        snprintf(full_path, MAXLINE, "%s/%s", dir, cmd);  	// Form absolute path

        							// Check if the file exists, is executable
        if (access(full_path, X_OK) == 0) {
            pid_t pid = fork();
            if (pid == 0) {
                execv(full_path, (char *[]) {cmd, NULL});	// In the child process, it executes the command
                perror("execv failed");  // If execv fails
                exit(1);
            } else if (pid > 0) {
                						// In the parent process, it should wait for the child to finish
                waitpid(pid, NULL, 0);
                return 1;  					// Command is found and executed
            } else {
                perror("fork failed");
                return 0;
            }
        }

        
        dir = strsep(&path_copy, ":");				// Move to the next directory in PATH
    }

    
    printf("Command '%s' not found\n", cmd);			// print Command not found in any directory in PATH
    return 0;
}


int main(int argc, char *argv[], char *envp[]) {		//  test environment commands
    char cmdLine[MAXLINE], **cmdArg;
    int status, i, debug;
    pid_t pid;

 copy_environ(envp);

    debug = 0;							//debug
    i = 1;
    while (i < argc) {
        if (!strcmp(argv[i], "-d"))
            debug = 1;
        i++;
    }

    while ((1)) {
        printf("bsh> ");                      			//prompt
        fgets(cmdLine, MAXLINE, stdin);       			//get a line from keyboard
        cmdLine[strlen(cmdLine) - 1] = '\0';  			//strip '\n'
        add_to_history(cmdLine);
	cmdArg = parseCmd(cmdLine);

        if (debug) {						//debug
            i = 0;
            while (cmdArg[i] != NULL) {
                printf("\t%d (%s)\n", i, cmdArg[i]);
                i++;
            }
        }

       
        if (strcmp(cmdArg[0], "exit") == 0) {			// Built-in command exit
            if (debug)
                printf("exiting\n");
            break;
        }
        
        else if (strcmp(cmdArg[0], "env") == 0) {		// Built-in command env
            command_environ();
        }
        else if (strcmp(cmdArg[0], "setenv") == 0) {		// Built-in command setenv
            if (cmdArg[1] != NULL && cmdArg[2] != NULL) {
                command_setenv(cmdArg[1], cmdArg[2]);
            } else {
                printf("try: setenv <variable> <value>\n");
            }
        }
        
        else if (strcmp(cmdArg[0], "unsetenv") == 0) {		// Built-in command unsetenv
            if (cmdArg[1] != NULL) {
                command_unsetenv(cmdArg[1]);
            } else {
                printf("try: unsetenv <variable>\n");
            }
        }
      
        else if (strcmp(cmdArg[0], "cd") == 0) {		// Built-in command cd
            if (cmdArg[1] != NULL) {
                command_changedirectory(cmdArg[1]);
            } else {
                command_changedirectory(NULL);  // Default to home directory if no argument
            }
        }
        
        else if (strcmp(cmdArg[0], "history") == 0) {		// Built-in command history
            command_history();
        }
        
        else {		// External command execution using PATH search
            if (debug)
                printf("calling search_execute()\n");
            if (!search_execute(cmdArg[0])) {
                printf("no such command (%s)\n", cmdArg[0]);
            }
        }

       
        i = 0;		// Clean up memory before running the next command
        while (cmdArg[i] != NULL)
            free(cmdArg[i++]);
        free(cmdArg);
    }

    return 0;
}
