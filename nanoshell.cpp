#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>

#define SIZE 1000000
#define MAX_VARS 100
#define MAX_VAR_LEN 50
#define MAX_VAL_LEN 200
#define TEST_MODE 1

typedef struct {
    char variable[MAX_VAR_LEN];
    char value[MAX_VAL_LEN];
} Variable;

Variable variables[MAX_VARS];
int var_count = 0;
int last_status = 0;

int isValidVariableAssignment(const char *input) {
    const char *equals = strchr(input, '=');
    if (equals == NULL) return 0;
    if (!isalpha(*input) && *input != '_') return 0;
    for (const char *ptr = input; ptr < equals; ptr++) {
        if (!isalnum(*ptr) && *ptr != '_') {
            return 0;
        }
    }
    return 1;
}

void trim_whitespace(char *str) {
    char *start = str;
    while (isspace((unsigned char)*start)) start++;
    if (*start == 0) {
        *str = 0;
        return;
    }

    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';

    if (start != str) memmove(str, start, strlen(start) + 1);
}

void expand_variables(char *input) {
    char buffer[SIZE] = {0};
    char *ptr = input;
    char *out = buffer;

    while (*ptr) {
        if (*ptr == '$') {
            char var_name[MAX_VAR_LEN] = {0};
            char *var_ptr = var_name;
            ptr++;
            while (isalnum(*ptr) || *ptr == '_') {
                *var_ptr++ = *ptr++;
            }
            *var_ptr = '\0';

            int found = 0;
            for (int i = 0; i < var_count; i++) {
                if (strcmp(variables[i].variable, var_name) == 0) {
                    strcpy(out, variables[i].value);
                    out += strlen(variables[i].value);
                    found = 1;
                    break;
                }
            }
            if (!found) {
                char *env_val = getenv(var_name);
                if (env_val) {
                    strcpy(out, env_val);
                    out += strlen(env_val);
                }
            }
        } else {
            *out++ = *ptr++;
        }
    }
    *out = '\0';
    strcpy(input, buffer);
}

int nanoshell_main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    char input[SIZE];
    int suppress_output = 0;

    while (1) {
        if (!suppress_output) {
            printf("nano shell -> ");
            fflush(stdout);
        }

        if (fgets(input, SIZE, stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = '\0';
        trim_whitespace(input);
        if (*input == '\0') {
            continue;
        }

        // Variable assignment
        if (strchr(input, '=') != NULL && isValidVariableAssignment(input)) {
            char *equals = strchr(input, '=');
            *equals = '\0';
            char *var_name = input;
            char *value = equals + 1;
            trim_whitespace(var_name);
            trim_whitespace(value);

            int i;
            for (i = 0; i < var_count; i++) {
                if (strcmp(variables[i].variable, var_name) == 0) {
                    strncpy(variables[i].value, value, MAX_VAL_LEN);
                    break;
                }
            }
            if (i == var_count && var_count < MAX_VARS) {
                strncpy(variables[var_count].variable, var_name, MAX_VAR_LEN);
                strncpy(variables[var_count].value, value, MAX_VAL_LEN);
                var_count++;
            } else if (i == var_count) {
                if (!suppress_output) fprintf(stderr, "Maximum variables reached\n");
                last_status = 1;
            } else {
                last_status = 0;
            }
            continue;
        }

        expand_variables(input);

        // Built-in commands
        if (strncmp(input, "echo", 4) == 0 && (input[4] == ' ' || input[4] == '\0')) {
            char *text = input + 4;
            while (isspace(*text)) text++;

            char *token = strtok(text, " \t");
            while (token) {
                printf("%s", token);
                token = strtok(NULL, " \t");
                if (token) printf(" ");
            }
            printf("\n");
            last_status = 0;
        }
        else if (strcmp(input, "exit") == 0) {
            if (!suppress_output) printf("Good Bye\n");
            break;
        }
        else if (strncmp(input, "cd ", 3) == 0) {
            char *directory = input + 3;
            trim_whitespace(directory);
            if (chdir(directory) == -1) {
                if (!suppress_output)
                    fprintf(stderr, "cd: %s: %s\n", directory, strerror(errno));
                last_status = 1;
            } else {
                last_status = 0;
            }
        }
        else if (strcmp(input, "pwd") == 0) {
            char cwd[SIZE];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
                last_status = 0;
            } else {
                if (!suppress_output) perror("pwd");
                last_status = 1;
            }
        }
        else if (strncmp(input, "export ", 7) == 0) {
            char *assignment = input + 7;
            char *equal_sign = strchr(assignment, '=');
            if (!equal_sign || equal_sign == assignment || *(equal_sign + 1) == '\0') {
                if (!suppress_output) fprintf(stderr, "Invalid export syntax\n");
                last_status = 1;
                continue;
            }
            *equal_sign = '\0';
            if (setenv(assignment, equal_sign + 1, 1)) {
                if (!suppress_output) perror("export");
                last_status = 1;
            } else {
                last_status = 0;
            }
        }
        else if (strcmp(input, "printenv") == 0) {
            extern char **environ;
            for (char **env = environ; *env != NULL; env++) {
                printf("%s\n", *env);
            }
            last_status = 0;
        }
        else if (strcmp(input, "test_mode_on") == 0) {
            suppress_output = 1;
            last_status = 0;
        }
        else if (strcmp(input, "test_mode_off") == 0) {
            suppress_output = 0;
            last_status = 0;
        }
        else {
            char command_name[SIZE];
            strcpy(command_name, input);
            char *space = strchr(command_name, ' ');
            if (space) *space = '\0';

            pid_t pid = fork();
            if (pid == -1) {
                if (!suppress_output) perror("fork");
                last_status = 1;
                continue;
            }
            else if (pid == 0) {
                char *args[SIZE / 2];
                char *token = strtok(input, " ");
                int index = 0;
                while (token != NULL) {
                    args[index++] = token;
                    token = strtok(NULL, " ");
                }
                args[index] = NULL;

                execvp(args[0], args);
                if (!suppress_output) {
                    if (errno == ENOENT) {
                        fprintf(stderr, "%s: command not found\n", command_name);
                    } else {
                        perror(command_name);
                    }
                }
                exit(127);
            }
            else {
                int status;
                waitpid(pid, &status, 0);
                last_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
            }
        }
    }

    return last_status;
}
