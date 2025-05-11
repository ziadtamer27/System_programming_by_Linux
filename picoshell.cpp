#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <errno.h>

#define MAX_ARGS 128  
#define BUF_SIZE 2048 

int picoshell_main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    char buf[BUF_SIZE];  
    char cwd[PATH_MAX];
    int last_status = 0;  

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Pico shell prompt > ");
            fflush(stdout);
        } else {
            perror("getcwd");
            return last_status;  
        }

        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            return last_status;  
        }

        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
        }

        if (strlen(buf) == 0) {
            continue;  
        }

        char **args = (char **)malloc(MAX_ARGS * sizeof(char *));
        if (args == NULL) {
            perror("malloc");
            return last_status;  
        }

        int arg_count = 0;
        char *token = strtok(buf, " ");
        while (token != NULL && arg_count < MAX_ARGS - 1) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;

        if (strcmp(args[0], "exit") == 0) {
            free(args);
            printf("Good Bye\n");
            return last_status;  
        } else if (strcmp(args[0], "echo") == 0) {
            for (int i = 1; args[i] != NULL; i++) {
                printf("%s%s", args[i], args[i + 1] ? " " : "");
            }
            printf("\n");
            last_status = 0;  
        } else if (strcmp(args[0], "pwd") == 0) {
            printf("%s\n", cwd);
            last_status = 0;  
        } else if (strcmp(args[0], "cd") == 0) {
            char *path = args[1];
            if (path == NULL || strcmp(path, "~") == 0) {
                path = getenv("HOME");
                if (path == NULL) {
                    fprintf(stderr, "cd: HOME not set\n");
                    last_status = 1;
                    free(args);
                    continue;
                }
            }
            if (chdir(path) != 0) {
                fprintf(stderr, "cd: %s: %s\n", path, strerror(errno));
                last_status = 1;
            } else {
                last_status = 0;
            }
        } else {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                last_status = 1;
            } else if (pid == 0) {
                execvp(args[0], args);
                if (errno == ENOENT) {
                    fprintf(stderr, "%s: command not found\n", args[0]);
                    exit(1);
                } else {
                    perror(args[0]);
                    exit(1);
                }
            } else {
                waitpid(pid, &last_status, 0);
                if (WIFEXITED(last_status)) {
                    last_status = WEXITSTATUS(last_status);
                } else {
                    last_status = 1;
                }
            }
        }

        free(args);
    }
}
