#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h.h>
#include <fcntl.h>
#include <stdlib.h>

// Whether to use system calls or library functions
bool syscalls = true;

void flags_to_char(const int flags, char* char_flags) {
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
}

// returns error code
int file_open(const char* path, const int flags, void** file) {
    if(syscalls) {
        int result = open(path, flags);
        if(result < 0) {
            file = NULL;
            return result;
        } else {
            *file = malloc(sizeof(int));
            *(int*) file = result;
        }
    } else {
        char* flags_char = calloc(3, sizeof(char));
        flags_to_char(flags, flags_char);
        *(FILE**) file = fopen(path, flags_char);
        if(*file == 0) {
            return -1; // Open error
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