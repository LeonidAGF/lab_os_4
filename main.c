#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

void plus_char(char **str, char data, long *capacity, long *length) {
    if (*length + 2 > *capacity) {
        *capacity *= 2;
        char *new_str = (char *) realloc(*str, (*capacity) * sizeof(char));
        if (new_str == NULL) {
            exit(1);
        }
        *str = new_str;
    }
    (*str)[*length] = data;
    (*length)++;
    (*str)[*length] = '\0';
}

int main() {
    long capacity_name = 16;
    long len_name = 0;
    char *filename = (char *) malloc(capacity_name * sizeof(char));
    char c;
    long capacity_line = 16;
    long len_line = 0;
    char *line;

    if (!filename) {
        return 1;
    }

    printf("Имя файла");
    c = getchar();

    while (c != '\n' && c != EOF) {
        plus_char(&filename, c, &capacity_name, &len_name);
        c = getchar();
    }

    if (len_name == 0) {
        free(filename);
        return 1;
    }

    int pipe1[2];
    int pipe2[2];

    if (pipe(pipe1) < 0 || pipe(pipe2) < 0) {
        free(filename);
        return 1;
    }

    long long pid = fork();

    if (pid < 0) {
        free(filename);
        return 1;
    }

    if (pid == 0) {
        close(pipe1[1]);
        close(pipe2[0]);

        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);

        dup2(pipe2[1], 3);
        close(pipe2[1]);

        char fd_str[2] = "3";

        execl("./child", "child", filename, fd_str, NULL);

        exit(1);
    } else {
        close(pipe1[0]);
        close(pipe2[1]);

        char response[4];
        printf("Введите числа:\n");

        capacity_line = 16;
        len_line = 0;
        line = (char *) malloc(capacity_line * sizeof(char));

        c = getchar();
        while (c != '\n' && c != EOF) {
            plus_char(&line, c, &capacity_line, &len_line);
            c = getchar();
        }

        plus_char(&line, '\n', &capacity_line, &len_line);
        if (write(pipe1[1], line, len_line) < 0) {
            free(line);
            return 1;
        }

        long long bytes_read = read(pipe2[0], response, 3);

        if (bytes_read > 0) {
            response[3] = '\0';
            if (strncmp(response, "ERR", 3) == 0) {
                free(line);
                return 1;
            }
        } else {
            free(line);
            return 1;
        }

        free(line);
        close(pipe1[1]);
        close(pipe2[0]);
        waitpid(pid, NULL, 0);
        free(filename);
    }
    return 0;
}