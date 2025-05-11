#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

extern char **environ;

#define size 15000
#define pwd 1000

typedef struct {
    char *name;
    char *value;
} ShellVar;

int microshell_main(int argc, char *argv[]) {
    char buf[size], pwdbuf[pwd];
    ShellVar *variables = NULL;
    int v = 0;
    int last_status = 0;

    while (1) {
        printf("FSP > ");
        fflush(stdout);

        if ((fgets(buf, size, stdin)) == NULL) {
            for (int i = 0; i < v; i++) {
                free(variables[i].name);
                free(variables[i].value);
            }
            free(variables);
            exit(last_status);
        }

        buf[strlen(buf) - 1] = 0;

        if (strlen(buf) == 0)
            continue;

        char **c = NULL;
        int j = 0;
        int k = 0;
        int buflen = strlen(buf);
        char temp[buflen + 1];
        memset(temp, 0, sizeof(temp));

        for (int i = 0; i <= buflen; ++i) {
            if (buf[i] != ' ' && buf[i] != '\0') {
                temp[k++] = buf[i];
            } else {
                if (k > 0) {
                    temp[k] = '\0';
                    c = (char **) realloc(c, sizeof(char *) * (j + 1));
                    c[j] = (char *) malloc(strlen(temp) + 1);
                    strcpy(c[j], temp);
                    j++;
                    k = 0;
                }
            }
        }

        c = (char **) realloc(c, sizeof(char *) * (j + 1));
        c[j] = NULL;

        if (j == 0) {
            free(c);
            continue;
        }

        for (int i = 0; i < j; i++) {
            char *dollar = strchr(c[i], '$');
            if (dollar != NULL) {
                char *varname = dollar + 1;
                int found = 0;
                for (int vi = 0; vi < v; vi++) {
                    if (strcmp(variables[vi].name, varname) == 0) {
                        size_t prefix_len = dollar - c[i];
                        char newval[strlen(c[i]) + strlen(variables[vi].value)];
                        strncpy(newval, c[i], prefix_len);
                        newval[prefix_len] = '\0';
                        strcat(newval, variables[vi].value);
                        c[i] = (char*) realloc(c[i], strlen(newval) + 1);
                        strcpy(c[i], newval);
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    *dollar = '\0';
                }
            }
        }

        
        int std_in = dup(0), std_out = dup(1), std_err = dup(2);
        int redir_error = 0;

        for (int i = 0; i < j; i++) {
            if (strcmp(c[i], "<") == 0) {
                int in_fd = open(c[i+1], O_RDONLY);
                if (in_fd < 0) {
                    fprintf(stderr, "cannot access %s: No such file or directory\n", c[i + 1]);
                    last_status = 1;
                    redir_error = 1;
                    break;
                }
                dup2(in_fd , 0);
                close(in_fd);
                free(c[i]);
                free(c[i+1]);
                for (int k = i; k + 2 <= j; ++k) {
                    c[k] = c[k + 2];
                }
                j -= 2;
                c[j] = NULL;
                c[j + 1] = NULL; 
                i--;
            }
            else if(strcmp(c[i], ">") == 0){
                int out_fd = open(c[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (out_fd < 0) {
                    fprintf(stderr, "%s: Permission denied\n", c[i + 1]);
                    last_status = 1;
                    redir_error = 1;
                    break;
                }
                dup2(out_fd , 1);
                close(out_fd);
                free(c[i]);
                free(c[i+1]);
                for (int k = i; k + 2 <= j; ++k) {
                    c[k] = c[k + 2];
                }
                j -= 2;
                c[j] = NULL;
                c[j + 1] = NULL;  
                i--;
            }
            else if(strcmp(c[i], "2>") == 0){
                int err_fd = open(c[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (err_fd < 0) {
                    fprintf(stderr, "cannot access %s: No such file or directory\n", c[i + 1]);
                    last_status = 1;
                    redir_error = 1;
                    break;
                }
                dup2(err_fd , 2);
                close(err_fd);
                free(c[i]);
                free(c[i+1]);
                for (int k = i; k + 2 <= j; ++k) {
                    c[k] = c[k + 2];
                }
                j -= 2;
                c[j] = NULL;
                c[j + 1] = NULL;
                i--;
            }
        }

        if (redir_error) {
            dup2(std_in, 0);
            dup2(std_out, 1);
            dup2(std_err, 2);
            close(std_in);
            close(std_out);
            close(std_err);
            for (int i = 0; i < j; i++) free(c[i]);
            free(c);
            continue;
        }

        if(strchr(c[0],'=') != NULL){
            for(int i=0;c[0][i] !=0;++i){
                if(c[0][i] =='=' && c[0][i-1] !=' ' && c[0][i+1] !=' ' && (j == 1 || c[1][0] ==0)) {
                    c[0][i] = '\0';
                    char *name = c[0];
                    char *value = c[0] + i + 1;

                    int found = 0;
                    for (int vi = 0; vi < v; vi++) {
                        if (strcmp(variables[vi].name, name) == 0) {
                            free(variables[vi].value);
                            variables[vi].value = strdup(value);
                            found = 1;
                            break;
                        }
                    }

                    if (!found) {
                        variables = (ShellVar *) realloc(variables, sizeof(ShellVar) * (v + 1));
                        variables[v].name = strdup(name);
                        variables[v].value = strdup(value);
                        v++;
                    }
                    last_status = 0;
                    break;
                }
                else if(c[0][i] =='=') {
                    printf("Invalid command\n");
                    last_status = 1;
                    break;
                }
            }
            for (int i = 0; i < j; i++) free(c[i]);
            free(c);
            continue;
        }
        else if ((strcmp(c[0], "exit")) == 0) {
            printf("Good Bye\n");
            for (int i = 0; i < j; i++)
                free(c[i]);
            free(c);
            for (int i = 0; i < v; i++) {
                free(variables[i].name);
                free(variables[i].value);
            }
            free(variables);
            return last_status;
        }
        else if ((strcmp(c[0], "pwd")) == 0) {
            if (getcwd(pwdbuf, pwd) == NULL) {
                for (int i = 0; i < j; i++)
                    free(c[i]);
                free(c);
                last_status = 1;
            } else {
                printf("%s\n", pwdbuf);
                last_status = 0;
            }
        }
        else if ((strcmp(c[0], "cd")) == 0) {
            if (chdir(c[1]) == -1) {
                printf("cd: %s: No such file or directory\n", c[1]);
                last_status = 1;
            } else {
                last_status = 0;
            }
        }
        else if ((strcmp(c[0], "echo")) == 0) {
            for (int i = 1; i < j; i++) {
                printf("%s", c[i]);
                if (i < j - 1)
                    printf(" ");
            }
            printf("\n");
            last_status = 0;
        }
        else if ((strcmp(c[0], "export")) == 0) {
            int found = 0;
            for (int vi = 0; vi < v; vi++) {
                if (strcmp(variables[vi].name, c[1]) == 0) {
                    setenv(variables[vi].name, variables[vi].value, 1);
                    last_status = 0;
                    found = 1;
                    break;
                }
            }
            if (!found) last_status = 1;
        }
        else {
            pid_t pid = fork();
            if (pid > 0) {
                int wstatus;
                wait(&wstatus);
                if (WIFEXITED(wstatus)) {
                    last_status = WEXITSTATUS(wstatus);
                } else {
                    last_status = 1;
                }
            }
            else if (pid == 0) {
                execvp(c[0], c);
                fprintf(stderr,"%s: command not found\n", c[0]);
                exit(127);
            }
        }

        
        dup2(std_in, 0);
        dup2(std_out, 1);
        dup2(std_err, 2);
        close(std_in);
        close(std_out);
        close(std_err);

        for (int i = 0; i < j; i++) free(c[i]);
        free(c);
    }

    return last_status;
}
