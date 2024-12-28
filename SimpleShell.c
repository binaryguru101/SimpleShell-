#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>

#define ENTRYSIZE 256
#define HISTORY  100

typedef struct {
    char *command;  // Store command as a single string
    time_t starting;
    time_t ending;
    double duration;
    pid_t PIDPRO;
    int ExitStatus; 
    char *Status;  //  char pointer
} Command;

typedef struct {
    Command *record;  // Pointer to array of Commands
    int count;
} History;


//iniliaze history by allocating the memoory
void Initialize_History(History *his) {
    his->record = malloc(HISTORY * sizeof(Command));
    if (!his->record) {
        perror("Failed to allocate memory for history");
        exit(EXIT_FAILURE);
    }
    his->count = 0;
}

//deleting the history and freeing the space

void Delete_History(History *his) {
    for (int i = 0; i < his->count; i++) {
        free(his->record[i].command);
        free(his->record[i].Status);  // Free the status string
    }
    free(his->record);
}

//adding the commands pid time usage to the history 

void add_to_history(History *his, const char *command, pid_t pid, time_t start_time, time_t end_time, const char *status) {
    if (his->count >= HISTORY) {
        fprintf(stderr, "Command history is full. Cannot add new command.\n");
        return;
    }

    his->record[his->count].command = malloc(strlen(command) + 1);
    if (!his->record[his->count].command) {
        perror("Failed to allocate memory for command");
        return;
    }
    strcpy(his->record[his->count].command, command);
    
    his->record[his->count].starting = start_time;
    his->record[his->count].ending = end_time;
    his->record[his->count].duration = difftime(end_time, start_time);
    his->record[his->count].PIDPRO = pid;
    his->record[his->count].Status = malloc(strlen(status) + 1);
    if (!his->record[his->count].Status) {
        perror("Failed to allocate memory for status");
        free(his->record[his->count].command);
        return;
    }
    strcpy(his->record[his->count].Status, status); // Set the status

    his->count++;
}



void display_history(const History *his) {
    for (int i = 0; i < his->count; i++) {
        printf("Command: %s, PID: %d, Duration: %.2f seconds, Status: %s\n", 
               his->record[i].command, 
               his->record[i].PIDPRO, 
               his->record[i].duration, 
               his->record[i].Status);
    }
}

//read the user input and parses into token with the tokenize function

char *read_user_input() {
    char *input = malloc(ENTRYSIZE);
    if (!input) {
        perror("Failed to allocate memory for user input");
        return NULL;
    }
    fgets(input, ENTRYSIZE, stdin);
    input[strcspn(input, "\n")] = 0;  // Remove newline character
    return input;
}

char **tokenize(char *command) {
    char **tokens = malloc(ENTRYSIZE * sizeof(char *));
    char *token;
    int position = 0;

    token = strtok(command, " ");
    while (token != NULL) {
        tokens[position++] = token;
        token = strtok(NULL, " ");
    }
    tokens[position] = NULL;  // Null-terminate the array to indicate that a specific commands end there
    return tokens;
}

void execute_pipe_commands(char *input, pid_t *child_pid) {
    int pipe_fd[2];
    pid_t pid1, pid2;
    char *cmd1, *cmd2;

    cmd1 = strtok(input, "|");
    cmd2 = strtok(NULL, "|");

    if (cmd2 == NULL) {
        fprintf(stderr, "Invalid command format.\n");
        return;
    }

    // Create a pipe
    if (pipe(pipe_fd) == -1) {
        perror("Pipe failed");
        return;
    }

    // Fork first child for the first command
    pid1 = fork();
    if (pid1 == 0) {
        // First child: execute cmd1
        close(pipe_fd[0]);  // Close unused read end
        dup2(pipe_fd[1], STDOUT_FILENO);  // Redirect stdout to pipe
        close(pipe_fd[1]);

        char **args1 = tokenize(cmd1);
        execvp(args1[0], args1);
        perror("Command execution failed");
        exit(EXIT_FAILURE);
    } else if (pid1 > 0) {
        // Fork second child for the second command
        pid2 = fork();
        if (pid2 == 0) {
            // Second child to execute cmd2
            close(pipe_fd[1]);  // Close unused write end
            dup2(pipe_fd[0], STDIN_FILENO);  // Redirect stdin to pipe
            close(pipe_fd[0]);

            char **args2 = tokenize(cmd2);
            execvp(args2[0], args2);
            perror("Command execution failed");
            exit(EXIT_FAILURE);
        } else {
            // Parent to wait for both children
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
    } else {
        perror("Fork failed");
    }
}

void sigint_handler(int sig) {
    printf("\nCaught signal %d  use exit to quitt he shell.\n", sig);
    printf("> ");  
    fflush(stdout);  // Flush to make sure the prompt appears immediately
}

void terminate_shell(History *history) {
    printf("\nShell terminated. Displaying command execution details:\n");
    for (int i = 0; i < history->count; i++) {
        printf("Command: %s, PID: %d, Start: %s, Duration: %.2f seconds, Status: %s\n",
               history->record[i].command, 
               history->record[i].PIDPRO, 
               ctime(&(history->record[i].starting)),
               history->record[i].duration,
               history->record[i].Status);
    }
    Delete_History(history);  // Clean up history before exiting
    exit(0);
}

void shell_loop() {
    History history;
    Initialize_History(&history);

    while (true) {
        printf("> ");  
        char *input = read_user_input();
        if (!input) break;  // Check for memory allocation failure

        // Check if the input contains a pipe
        if (strchr(input, '|')) {
            execute_pipe_commands(input, NULL);
        } else {
            char **args = tokenize(input);
            if (args[0] == NULL) {
                free(input);
                free(args);
                continue;  // Skip if input is empty
            }

            // use exit and history to exit and view history
            if (strcmp(args[0], "exit") == 0) {
                terminate_shell(&history);
            }

            if (strcmp(args[0], "history") == 0) {
                display_history(&history);
                free(input);
                free(args);
                continue;  // Skip to the next iteration
            }

            // Check if the command should run in the background
            bool is_background = false;
            int arg_len = 0;
            while (args[arg_len] != NULL) arg_len++;
            if (arg_len > 0 && strcmp(args[arg_len - 1], "&") == 0) {
                is_background = true;
                args[arg_len - 1] = NULL;  // Remove the '&' from the command arguments and replace with NULL
            }

            time_t start_time = time(NULL);  
            pid_t child_pid;
            int exit_status;

            pid_t pid = fork();  // Fork the process
            if (pid == 0) {
                // Child process executes the command
                execvp(args[0], args);
                perror("Command execution failed");
                exit(EXIT_FAILURE);
            } else if (pid > 0) {
                // Parent process
                child_pid = pid;  // Record the child's PID

                if (!is_background) {
                    // Wait for the child if not running in the background
                    waitpid(pid, &exit_status, 0);
                    time_t end_time = time(NULL);
                    add_to_history(&history, input, child_pid, start_time, end_time, "COMPLETED");
                } else {
                    // Background process, don't wait
                    printf("Command running in background with PID: %d\n", pid);
                    add_to_history(&history, input, child_pid, start_time, 0, "RUNNING");  // Initially set to RUNNING
                }
            } else {
                perror("Fork failed");
            }
        }

        free(input);  // Free the input string
    }

    Delete_History(&history);
}


int main() {
    signal(SIGINT, sigint_handler);  
    shell_loop();  
    return 0;
}
