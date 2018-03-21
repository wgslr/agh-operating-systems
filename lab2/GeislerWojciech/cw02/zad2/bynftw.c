
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <string.h>
#include <zconf.h>

#define __USE_XOPEN_EXTENDED

#include <ftw.h>

#define __USE_XOPEN

#include <time.h>


typedef struct dirent dirent;

const char *PERMISSIONS = "rwxrwxrwx";

time_t target_time;
char comparator;

bool is_absolute(const char *path) {
    return path[0] == '/';
}

time_t str_to_time(const char *str) {
    struct tm time;

    strptime(str, "%Y-%m-%d", &time);
    return mktime(&time);
}

char *mode_to_str(mode_t mode) {
    char *result = calloc(10, sizeof(char));
    memset(result, '-', 9);
    unsigned mask = 1 << 8;
    int pos = 0;
    while(mask) {
        if(mode & mask) {
            result[pos] = PERMISSIONS[pos];
        }
        mask = mask >> 1;
        ++pos;
    }
    return result;
}

// size of buf must be at least strlen(path1) + strlen(path2) + 2;
char *join_paths(const char *path1, const char *path2) {
    char *buf;
    int len1 = strlen(path1);
    int len2 = strlen(path2);

    if(is_absolute(path2)) {
        // Absolute path overrides path1
        buf = malloc(len2 + 1);
        strcpy(buf, path2);
    } else {
        char *sep = "/";
        buf = malloc(len1 + len2 + 2);
        if(path1[len1 - 1] == '/') {
            sep = "";
        }
        sprintf(buf, "%s%s%s", path1, sep, path2);
    }
    return buf;
}

char *absolute(const char *path) {
    if(is_absolute(path)) {
        // copy for consistency
        char *copy = malloc(strlen(path) + 1);
        strcpy(copy, path);
        return copy;
    }

    int buf_size = 64;
    char *workdir = calloc(buf_size, sizeof(char));
    while(getcwd(workdir, buf_size) == 0) {
        buf_size *= 2;
        workdir = realloc(workdir, buf_size);
    }
    char *result = join_paths(workdir, path);
    free(workdir);
    return result;
}


void print_file(const char *path, const struct stat *stats) {
//    fprintf(stderr, "Path: %s\n", path);
    char *name = absolute(path);
    long size = stats->st_size;
    struct timespec mtime = stats->st_mtim;
    time_t time = mtime.tv_sec;
    char *mode = mode_to_str(stats->st_mode);
    char *time_str = ctime(&time);

    // specify time length to strip newline character
    printf("%s\t%10ld\t%.*s\t%s\n", mode, size, (int) strlen(time_str) - 1, time_str, name);

    free(mode);
    free(name);
}


bool is_time_allowed(struct stat *stats) {
    time_t file_days = stats->st_mtim.tv_sec / (3600 * 24);
    time_t target_days = target_time / (3600 * 24);
    long diff = file_days - target_days;
    return (comparator == '<' && diff < 0)
           || (comparator == '=' && diff == 0)
           || (comparator == '>' && diff > 0);
}


void display_dir(const char *path) {
    DIR *dir = opendir(path);
    dirent *entry;
    struct stat *stats = calloc(1, sizeof(struct stat));

    if(dir == NULL) {
        fprintf(stderr, "Error opening dir %s\n", path);
        exit(1);
    }


    // print normal files
    while((entry = readdir(dir)) != NULL) {
        char *filepath = join_paths(path, entry->d_name);
        lstat(filepath, stats);

        if(S_ISREG(stats->st_mode) && is_time_allowed(stats)) {
            print_file(filepath, stats);
        }
        free(filepath);
    }

    rewinddir(dir);

    while((entry = readdir(dir)) != NULL) {
        char *filepath = join_paths(path, entry->d_name);
        lstat(filepath, stats);
        if(S_ISDIR(stats->st_mode) &&
           strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) { // prevent recursing to the same dir
            display_dir(filepath);
        }
        free(filepath);
    }

    free(stats);
    closedir(dir);
}

int nftw_callback(const char *path, const struct stat *stats, int typeflag, struct FTW *ftwbuf) {
    (void)ftwbuf; // silence unsued variable warning
    if(typeflag == FTW_F && is_time_allowed(stats)) {
        print_file(path, stats);
    }
    return 0;
}

void display_dir_nftw(const char *path) {
    nftw(path, &nftw_callback, 512, 0);

}

int main(int argc, char *argv[]) {
    if(argc < 3) {
        fprintf(stderr, "Not enough arguments\n");
        exit(1);
    }
    const char *path = argv[1];
    comparator = argv[2][0];
    const char *date = argv[3];
    target_time= str_to_time(date);

    printf("Using opendir:\n");
    display_dir(path);

    printf("Using nftw:\n");
    display_dir_nftw(path);

    return 0;
}

