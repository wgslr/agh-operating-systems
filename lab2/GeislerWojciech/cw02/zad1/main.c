#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>

typedef struct File {
    int fd;
    FILE* handle;
    bool syscall;
} File;

// Converts a subset of open() flags to fopen() mode
// Returned array should be free-ed
char* flags_to_char(const int flags) {
    char* char_flags = calloc(3, sizeof(char));
    if(flags & O_APPEND) {
        char_flags[0] = 'a';
        char_flags[1] = '\0';
    }
    if(flags & O_RDWR) {
        char_flags[0] = 'r';
        char_flags[1] = 'w';
        char_flags[2] = '\0';
    } else if(flags & O_WRONLY) {
        char_flags[0] = 'w';
        char_flags[1] = '\0';
    } else {
        // O_RDONLY == 0
        char_flags[0] = 'r';
        char_flags[1] = '\0';
    }
    return char_flags;
}

// returns 0 on error
File* file_open(const char* path, const int flags, bool syscall) {
    File* file = calloc(1, sizeof(File));
    file->syscall = syscall;
    if(syscall) {
        int result = open(path, flags, S_IRUSR | S_IWUSR | S_IRGRP);
        if(result < 0) {
            free(file);
            return NULL;
        } else {
            file->fd = result;
            return file;
        }
    } else {
        char* flags_char = flags_to_char(flags);
        file->handle = fopen(path, flags_char);
        free(flags_char);
        if(file->handle != NULL) {
            return file;
        } else {
            free(file);
            return NULL;
        }
    }
}

void file_close(File** file_ptr){
    if((*file_ptr)->syscall) {
        close((*file_ptr)->fd);
    } else {
        fclose((*file_ptr)->handle);
    }
    free(*file_ptr);
    *file_ptr = NULL;
}

// Returns number of read bytes
// file is either (int*) or (FILE*)
unsigned file_read(File* file, unsigned size, void* buf) {
    if(file->syscall) {
        return (unsigned) read(file->fd, buf, size);
    } else {
        // multiply by size since fread returns number of *blocks* read
        return (unsigned) (size * (fread(buf, size, 1, file->handle)));
    }
}

// Returns number of written bytes
int file_write(File* file, unsigned size, const void* content) {
    if(file->syscall) {
        return (int) write(file->fd, content, size);
    } else {
        return (int) fwrite(content, size, 1, file->handle);
    }
}

void generate(const char* path, const unsigned records, const unsigned record_size, bool syscalls) {
    File* urandom = file_open("/dev/urandom", O_RDONLY, syscalls);
    File* target = file_open(path, O_WRONLY | O_CREAT | O_TRUNC, syscalls);

    if(urandom == NULL || target == NULL) {
        fprintf(stderr, "Error opening file");
        exit(1);
    }

    char* buf = malloc(record_size);
    int bytes = 0;

    for(int i = 0; i < records; ++i){
        bytes = file_read(urandom, record_size, buf);
        assert(bytes == record_size);
        file_write(target, bytes, buf);
    }

    free(buf);
    file_close(&target);
    file_close(&urandom);
}


int main(int argc, char* argv[]) {
    if(argc < 4){
        fprintf(stderr, "Wrong number of arguments: %d\n", argc);
        return (2);
    }
    const char* path = argv[1];
    const int records = atoi(argv[2]);
    const int record_size = atoi(argv[3]);

    generate(path, records, record_size, false);

    return 0;
}