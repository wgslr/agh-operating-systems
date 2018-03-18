#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

typedef struct File {
    int fd;
    FILE* handle;
    bool syscall;
} File;

// Converts from open() flags to fopen() mode
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
    } else if(flags & O_RDONLY) {
        char_flags[0] = 'r';
        char_flags[1] = '\0';
    } else if(flags & O_WRONLY) {
        char_flags[0] = 'w';
        char_flags[1] = '\0';
    } else {
        char_flags[0] = '\0';
    }
    return char_flags;
}

// returns 0 on error
File* file_open(const char* path, const int flags, bool syscall) {
    File* file = calloc(1, sizeof(File));
    file->syscall = syscall;
    if(syscall) {
        int result = open(path, flags);
        if(result < 0) {
            free(result);
            return NULL;
        } else {
            file->fd = result;
            return file;
        }
    } else {
        char* flags_char = flags_to_char(flags);
        file->handle = fopen(path, flags_char);
        free(flags_char);
        if(file->handle != 0) {
            return file;
        } else {
            free(file);
            return NULL;
        }
    }
}

// Returns number of read bytes
// file is either (int*) or (FILE*)
unsigned file_read(void* file, unsigned count, unsigned size, void* buf) {
    if(syscalls) {
        int file_descr = *(int*) file;
        return (unsigned) read(file_descr, buf, count * size);
    } else {
        return (unsigned int) (size * fread(buf, size, count, (FILE*) file));
    }
}

void generate(const char* path, const unsigned records, const unsigned record_size) {
    for
}

int main() {
    printf("File! operations\n");
    return 0;
}