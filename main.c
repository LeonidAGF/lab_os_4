#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>

#define NAME "/lab4"

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


    size_t shm_size = sizeof(struct shared) + len_line + 1;
    shm_unlink(SHM_NAME);
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (fd < 0) {
        perror("shm_open");
        free(line); free(filename);
        return 1;
    }
    if (ftruncate(fd, shm_size) < 0) {
        perror("ftruncate");
        close(fd); shm_unlink(SHM_NAME);
        free(line); free(filename);
        return 1;
    }

    struct shared *shm = mmap(NULL, shm_size, PROT_READ | PROT_WRITE,
                              MAP_SHARED, fd, 0);
    close(fd);
    if (shm == MAP_FAILED) {
        perror("mmap");
        shm_unlink(SHM_NAME);
        free(line); free(filename);
        return 1;
    }

    memset(shm->response, 0, sizeof(shm->response));
    memcpy(shm->data, line, len_line + 1);

    char size_str[32];
    snprintf(size_str, sizeof(size_str), "%zu", shm_size);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        munmap(shm, shm_size); shm_unlink(SHM_NAME);
        free(line); free(filename);
        return 1;
    }

    if (pid == 0) {
        execl("./child", "child", filename, SHM_NAME, size_str, NULL);
        perror("execl");
        exit(1);
    }

    int status = 0;
    waitpid(pid, &status, 0);

    int error = 0;
    if (strncmp(shm->response, "ERR", 3) != 0 &&
        strncmp(shm->response, "OK\n", 3) != 0) {
        error = 1;                       /* ребёнок ничего не ответил */
    } else if (strncmp(shm->response, "ERR", 3) == 0) {
        error = 1;
    }

    munmap(shm, shm_size);
    shm_unlink(SHM_NAME);
    free(line);
    free(filename);

    return error ? 1 : 0;
}