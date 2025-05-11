#include <stdio.h>
#include <string.h>

#define SIZE 100

int femtoshell_main(int argc, char *argv[]) {
    char status = 0;
    char buf[SIZE];

    while (1) {
        printf("Welcome to my shell -> ");

        if (fgets(buf, SIZE, stdin) == NULL) {
            return status;
        }

        if (strncmp(buf, "echo ", 5) == 0) {
            memmove(buf, buf + 5, strlen(buf + 5) + 1);

            do {
                size_t len = strlen(buf);
                if (len > 0 && buf[len - 1] == '\n') {
                    buf[len - 1] = '\0';
                    printf("%s", buf);
                    break;
                } else {
                    printf("%s", buf);
                }
            } while (fgets(buf, SIZE, stdin) != NULL);

            printf("\n");
            status = 0;
        } else if (strcmp(buf, "exit\n") == 0) {
            printf("Good Bye\n");
            return 0;
        } else if (buf[0] == '\n') {
            continue;
        } else {
            printf("Invalid command\n");
            status = -1;
        }
    }

    return 0;
}
