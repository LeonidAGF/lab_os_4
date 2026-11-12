#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

char* NAME = "lab4";

struct shared {
    char response[4];
    char data[];
};

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
    int c;
    long capacity_line = 16;
    long len_line = 0;
    char *line;

    if (!filename) {
        return 1;
    }

    printf("Имя файла: ");
    c = getchar();

    while (c != '\n' && c != EOF) {
        plus_char(&filename, c, &capacity_name, &len_name);
        c = getchar();
    }

    if (len_name == 0) {
        free(filename);
        return 1;
    }

    printf("Введите числа:\n");
    line = (char *) malloc(capacity_line * sizeof(char));

    if (!line) {
        free(filename);
        return 1;
    }

    c = getchar();
    while (c != '\n' && c != EOF) {
        plus_char(&line, c, &capacity_line, &len_line);
        c = getchar();
    }
    plus_char(&line, '\n', &capacity_line, &len_line);

    size_t size = sizeof(struct shared) + len_line + 1;

    int fd = open(NAME, O_CREAT | O_RDWR | O_TRUNC, 0600);
    if (fd < 0) {
        free(line);
        free(filename);
        return 1;
    }
    if (ftruncate(fd, size) < 0) {
        close(fd);
        unlink(NAME);
        free(line);
        free(filename);
        return 1;
    }

    struct shared *m = mmap(NULL, size, PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
    close(fd);
    if (m == MAP_FAILED) {
        unlink(NAME);
        free(line);
        free(filename);
        return 1;
    }

    memcpy(m->data, line, len_line + 1);

    char size_str[32];
    snprintf(size_str, sizeof(size_str), "%zu", size);

    pid_t pid = fork();

    if (pid < 0) {
        munmap(m, size);
        unlink(NAME);
        free(line);
        free(filename);
        return 1;
    }

    if (pid == 0) {
        execl("./child", "child", filename, NAME, size_str, NULL);
        exit(1);
    }

    int status = 0;
    waitpid(pid, &status, 0);

    int error = 0;
    if (strncmp(m->response, "OK\n", 3) != 0) {
        error = 1;
    }

    munmap(m, size);
    unlink(NAME);
    free(line);
    free(filename);

    return 0;
}