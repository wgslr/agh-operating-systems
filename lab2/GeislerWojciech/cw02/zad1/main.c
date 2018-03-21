#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <assert.h>


typedef struct File {
    int fd;
    FILE *handle;
    bool syscall;
} File;

typedef struct timeval timeval;
typedef struct timespec timespec;

typedef struct timestamp {
    timeval system;
    timeval user;
    timeval real;
} timestamp;

void print_timediff(timeval start, timeval end, const char *description);

/** Timing functions **/

timestamp get_timestamp();

timeval timespec_to_timeval(timespec time);

void print_timing(timestamp start, timestamp end, const char *description);

// Converts a subset of open() flags to fopen() mode
// Returned array should be free-ed
char *flags_to_char(const int flags) {
    char *char_flags = calloc(3, sizeof(char));
    if((flags & O_RDWR) == O_RDWR) {
        if((flags & O_TRUNC) == O_TRUNC && (flags & O_CREAT) == O_CREAT) {
            char_flags[0] = 'w';
        } else if((flags & O_APPEND) == O_APPEND) {
            char_flags[0] = 'a';
        } else {
            char_flags[0] = 'r';
        }
        char_flags[1] = '+';
        char_flags[2] = '\0';
    } else if((flags & O_APPEND) == O_APPEND) {
        char_flags[0] = 'a';
        char_flags[1] = '\0';
    } else if((flags & O_WRONLY) == O_WRONLY) {
        char_flags[0] = 'w';
        char_flags[1] = '\0';
    } else {
        // Checked last as O_RDONLY == 0
        char_flags[0] = 'r';
        char_flags[1] = '\0';
    }
    return char_flags;
}

// returns 0 on error
File *file_open(const char *path, const int flags, bool syscall) {
    File *file = calloc(1, sizeof(File));
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
        char *flags_char = flags_to_char(flags);
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

void file_close(File **file_ptr) {
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
int file_read(const File *file, int size, void *buf) {
    if(file->syscall) {
        return (int) read(file->fd, buf, (size_t) size);
    } else {
        // multiply by size since fread returns number of *blocks* read
        return size * (int) fread(buf, (size_t) size, 1, file->handle);
    }
}

// Returns number of written bytes
int file_write(File *file, int size, const void *content) {
    if(file->syscall) {
        return (int) write(file->fd, content, (size_t) size);
    } else {
        return (int) (size * fwrite(content, (size_t) size, 1, file->handle));
    }
}


void file_seek(const File *file, const off_t offset, const int whence) {
    if(file->syscall) {
        lseek(file->fd, offset, whence);
    } else {
        fseek(file->handle, offset, whence);
    }
}


void copy(const char *path_from, const char *path_to,
          const int records, const int record_size, bool syscalls) {
    fprintf(stderr, "Copying %ux%u bytes from \"%s\" to \"%s\"\n", records, record_size, path_from, path_to);

    File *source = file_open(path_from, O_RDONLY, syscalls);
    File *target = file_open(path_to, O_WRONLY | O_CREAT | O_TRUNC, syscalls);
    long int copied_bytes = 0;

    if(source == NULL || target == NULL) {
        fprintf(stderr, "Error opening file");
        exit(1);
    }

    char *buf = malloc(record_size);

    for(int i = 0; i < records; ++i) {
        int bytes = file_read(source, record_size, buf);
        file_write(target, bytes, buf);
        copied_bytes += bytes;

        if(bytes < record_size) {
            fprintf(stderr, "File %s contained less than %u records. Copied %ld bytes in %d records", path_from, records, copied_bytes, i);
            break;
        }
    }

    free(buf);
    file_close(&target);
    file_close(&source);

}


void generate(const char *path, const int records, const int record_size, bool syscalls) {
    copy("/dev/urandom", path, records, record_size, syscalls);
}


void sort(const char *path, const int records, const int record_size, bool syscalls) {
    unsigned char *buffer = malloc((size_t) record_size);
    unsigned char *current = malloc((size_t) record_size);
    File *file = file_open(path, O_RDWR, syscalls);
    int pos = 0; // tracks position in file in record_size units

    if(file == NULL) {
        fprintf(stderr, "Error opening file %s\n", path);
        exit(1);
    }

    file_seek(file, record_size, SEEK_SET);
    ++pos;
    for(int i = 1; i < records; ++i) {
        int read = file_read(file, record_size, current);
        if(read != record_size) {
            fprintf(stderr, "Read %d bytes of %d record\n", read, i);
//            fprintf(stderr, "Unexpected and of file %s reading %dth record\n", path, i);
            exit(1);
        }
        // compensate the read
        file_seek(file, -record_size, SEEK_CUR);


        while(pos > 0) {
            file_seek(file, -record_size, SEEK_CUR);
            int bytes_read = file_read(file, record_size, buffer);
            assert(bytes_read == record_size);

            if(current[0] < buffer[0]) {
                // move record one position right
                int written = file_write(file, record_size, buffer);
                assert(written == record_size);

                // move to next record left
                file_seek(file, -2 * record_size, SEEK_CUR);
                --pos;
            } else {
                break;
            }
        }
        file_write(file, record_size, current);
        ++pos;

        // move forward to read next record
        file_seek(file, (i - pos + 1) * record_size, SEEK_CUR);
        pos += i - pos + 1;
    }

    file_close(&file);
    free(buffer);
    free(current);
}


// offset - number of arguments before size parameters
void parse_params(int offset, char *argv[], int *records, int *record_size, bool *syscalls) {
    *records = atoi(argv[offset + 1]);
    *record_size = atoi(argv[offset + 2]);
    if(strcmp(argv[offset + 3], "sys") == 0) {
        *syscalls = true;
    } else if(strcmp(argv[offset + 3], "lib") == 0) {
        *syscalls = false;
    } else {
        fprintf(stderr, "Incorrect argument \"%s\"\n", argv[offset + 3]);
        exit(1);
    }
}


int main(int argc, char *argv[]) {
    if(argc < 5) {
        fprintf(stderr, "Wrong number of arguments: %d\n", argc);
        return (2);
    }

    int records;
    int record_size;
    bool syscalls;
    char description[100];

    if(strcmp(argv[1], "generate") == 0) {
        const char *path = argv[2];
        parse_params(2, argv, &records, &record_size, &syscalls);
        generate(path, records, record_size, syscalls);
    } else if(strcmp(argv[1], "copy") == 0) {
        const char *from = argv[2];
        const char *to = argv[3];
        parse_params(3, argv, &records, &record_size, &syscalls);

        timestamp start = get_timestamp();
        copy(from, to, records, record_size, syscalls);
        timestamp end = get_timestamp();

        sprintf(description, "copy;\t%5d;\t%5d;\t%4s;", records, record_size, syscalls ? "sys" : "lib");
        print_timing(start, end, description);

    } else if(strcmp(argv[1], "sort") == 0) {
        const char *path = argv[2];
        parse_params(2, argv, &records, &record_size, &syscalls);

        timestamp start = get_timestamp();
        sort(path, records, record_size, syscalls);
        timestamp end = get_timestamp();

        sprintf(description, "sort;\t%5d;\t%5d;\t%4s;", records, record_size, syscalls ? "sys" : "lib");
        print_timing(start, end, description);
    } else {
        fprintf(stderr, "Unidentified argument \'%s\"\n", argv[1]);
    }

    return 0;
}

/** Timing functions **/

timestamp get_timestamp() {
    timestamp ts;
    struct rusage ru;
    struct timespec real;

    clock_gettime(CLOCK_REALTIME, &real);
    ts.real = timespec_to_timeval(real);

    getrusage(RUSAGE_SELF, &ru);
    ts.system = ru.ru_stime;
    ts.user = ru.ru_utime;
    return ts;
}


void print_timing(const timestamp start, const timestamp end, const char *description) {
    char *buf = calloc(10 + strlen(description), sizeof(char));
    sprintf(buf, "%s user\t", description);
    print_timediff(start.user, end.user, buf);
    sprintf(buf, "%s system\t", description);
    print_timediff(start.system, end.system, buf);
    sprintf(buf, "%s real\t", description);
    print_timediff(start.real, end.real, buf);
    free(buf);
}


timeval timespec_to_timeval(timespec time) {
    timeval ts;
    ts.tv_sec = time.tv_sec;
    ts.tv_usec = time.tv_nsec / 1000;
    return ts;
}


void print_timediff(timeval start, timeval end, const char *description) {
    long double diff_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000;
    printf("%11s (ms): %10.5Lf\n", description, diff_ms);
}
