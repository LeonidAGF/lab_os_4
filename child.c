#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void plus_char(char** str, char data, long* capacity, long* length) {
    if (*length + 2 > *capacity) {
        *capacity *= 2;
        char* new_str = (char*)realloc(*str, (*capacity) * sizeof(char));
        if (new_str == NULL) {
            exit(1);
        }
        *str = new_str;
    }
    (*str)[*length] = data;
    (*length)++;
    (*str)[*length] = '\0';
}

int main(int argc, char *argv[]) {

    if (argc < 3) {
        return 1;
    }

    const char *filename = argv[1];
    int pipe2 = atoi(argv[2]);
    FILE *file = fopen(filename, "w");
    long capacity = 16;
    long length = 0;
    char *ptr;
    char *endptr;
    float first_num;

    if (!file) {
        return 1;
    }

    char* str = (char*)malloc(capacity * sizeof(char));
    if (!str) {
        write(pipe2, "ERR", 3);
        fclose(file);
        return 1;
    }

    char c = fgetc(stdin);
    if (c == EOF) {
        free(str);
        fclose(file);
        return 0;
    }

    while (c != '\n' && c != EOF) {
        plus_char(&str, c, &capacity, &length);
        c = fgetc(stdin);
    }

    if (length == 0) {
        free(str);
        write(pipe2, "ERR", 3);
        fclose(file);
        return 1;
    }

    ptr = str;
    first_num = strtof(ptr, &endptr);

    if (ptr == endptr) {
        write(pipe2, "OK\n", 3);
        free(str);
        fclose(file);
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
        write(pipe2, "ERR", 3);
        free(str);
        fclose(file);
        exit(1);
    }

    fprintf(file, "%f\n", first_num);
    fclose(file);
    write(pipe2, "OK\n", 3);
    free(str);

    return 0;
}