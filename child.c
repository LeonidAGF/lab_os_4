#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct shared {
    char response[4];
    char data[];
};

int main(int argc, char *argv[]) {

    if (argc < 4) {
        return 1;
    }

    const char *filename = argv[1];
    const char *name = argv[2];
    size_t shm_size = (size_t) strtoul(argv[3], NULL, 10);

    int fd = shm_open(name, O_RDWR, 0600);
    if (fd < 0) {
        return 1;
    }
    struct shared *shm = mmap(NULL, shm_size, PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
    close(fd);
    if (shm == MAP_FAILED) {
        return 1;
    }

    FILE *file = fopen(filename, "w");
    if (!file) {
        memcpy(shm->response, "ERR", 3);
        munmap(shm, shm_size);
        return 1;
    }

    char *str = shm->data;
    char *ptr;
    char *endptr;
    float first_num;

    if (str[0] == '\0' || str[0] == '\n') {
        memcpy(shm->response, "ERR", 3);
        fclose(file);
        munmap(shm, shm_size);
        return 1;
    }

    ptr = str;
    first_num = strtof(ptr, &endptr);

    if (ptr == endptr) {
        memcpy(shm->response, "OK\n", 3);
        fclose(file);
        munmap(shm, shm_size);
        return 0;
    }

    ptr = endptr;
    int div_by_zero = 0;

    while (*ptr != '\0') {
        float current_num = strtof(ptr, &endptr);

        if (ptr == endptr) {
            ptr++;
            continue;
        }

        if (current_num == 0.0f) {
            div_by_zero = 1;
            break;
        }

        first_num /= current_num;
        ptr = endptr;
    }

    if (div_by_zero) {
        memcpy(shm->response, "ERR", 3);
        fclose(file);
        munmap(shm, shm_size);
        exit(1);
    }

    fprintf(file, "%f\n", first_num);
    fclose(file);
    memcpy(shm->response, "OK\n", 3);
    munmap(shm, shm_size);

    return 0;
}