
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
#include <sys/wait.h>


typedef struct dirent dirent;

const char *PERMISSIONS = "rwxrwxrwx";

time_t target_time = 0;
char comparator = '\0';

bool is_path_absolute(const char *path) {
    return path[0] == '/';
}

// Assummes format yyyy-mm-dd
time_t str_to_time(char *str) {
    str[4] = '\0',
            str[7] = '\0';
    struct tm time = {0};

    time.tm_year = atoi(&str[0]) - 1900;
    time.tm_mon = atoi(&str[5]) - 1;
    time.tm_mday = atoi(&str[8]);
    time.tm_hour = 12;

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
    size_t len1 = strlen(path1);
    size_t len2 = strlen(path2);

    if(is_path_absolute(path2)) {
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

char *mk_path_absolute(const char *path) {
    if(is_path_absolute(path)) {
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
    char *name = mk_path_absolute(path);
    long size = stats->st_size;
    struct timespec mtime = stats->st_mtim;
    time_t time = mtime.tv_sec;
    char *mode = mode_to_str(stats->st_mode);
    char *time_str = ctime(&time);

    // specify time length to strip newline character
    printf("%6d: %s\t%10ld\t%.*s\t%s\n", getpid(), mode, size, (int) strlen(time_str) - 1, time_str, path);

    free(mode);
    free(name);
}


bool is_time_allowed(const struct stat *stats) {
    time_t file_days = stats->st_mtim.tv_sec / (3600 * 24);
    time_t target_days = target_time / (3600 * 24);
    long int diff = file_days - target_days;


    return  (comparator == '<' && diff < 0)
                  || (comparator == '=' && diff == 0)
                  || (comparator == '>' && diff > 0);
}


void display_dir(const char *path) {
    printf("%6d: display_dir(%s) in pid %d %dsson\n", getpid(), path, getpid(), getppid());
    int pid = fork();
    if(pid == 0) {
        printf("%6d: scanning dir %s in pid %d %dsson\n", getpid(), path, getpid(), getppid());

        DIR *dir = opendir(path);
        dirent *entry = NULL;
        char *filepath = NULL;
        struct stat *stats = calloc(1, sizeof(struct stat));

        if(dir == NULL) {
            fprintf(stderr, "Error opening dir %s\n", path);
            exit(1);
        }

        // print normal files
        while((entry = readdir(dir)) != NULL) {
            filepath = join_paths(path, entry->d_name);
            lstat(filepath, stats);

            if(S_ISREG(stats->st_mode) && is_time_allowed(stats)) {
                print_file(filepath, stats);
            }
            free(filepath);
        }

        rewinddir(dir);

        // descend into subdirectories
        while((entry = readdir(dir)) != NULL) {
            filepath = join_paths(path, entry->d_name);
            lstat(filepath, stats);
            if(S_ISDIR(stats->st_mode) &&
               strcmp(entry->d_name, ".") != 0 &&
               strcmp(entry->d_name, "..") != 0) { // prevent recursing to the same dir
               display_dir(filepath); // wraca tu i kontynuuje!
            }
            free(filepath);
        }
        printf("%6d: scanned dir %s in pid %d %dsson\n", getpid(), path, getpid(), getppid());

        free(stats);
        closedir(dir);
        exit(0);
    } else {
        printf("%6d: wait() in pid %d %dsson\n", getpid(), getpid(), getppid());
        int status;
        int waited_pid = wait(&status);
        printf("%6d: child %d finished with status %d\n", getpid(), waited_pid, WEXITSTATUS(status));
        WEXITSTATUS(status);
    }
}

int nftw_callback(const char *path, const struct stat *stats, int typeflag, struct FTW *ftwbuf) {
    (void) ftwbuf; // silence unused variable warning
    if(typeflag == FTW_F && is_time_allowed(stats)) {
        print_file(path, stats);
    }
    return 0;
}

void display_dir_nftw(const char *path) {
    nftw(path, &nftw_callback, 512, 0);
}

int main(int argc, char *argv[]) {
    printf("%6d: main() \n", getpid());
    if(argc < 3) {
        fprintf(stderr, "Not enough arguments\n");
        exit(1);
    }
    const char *path = argv[1];
    comparator = argv[2][0];

    if(comparator != '>' && comparator != '<' && comparator != '=') {
        fprintf(stderr, "Comparison operator must be < = or >\n");
        exit(1);
    }

    target_time = str_to_time(argv[3]);

    printf("%6d: pre-display_dir(%s) in main in %d %dsson\n", getpid(), path, getpid(), getppid());
    display_dir(path);

//    printf("Using nftw:\n");
//    display_dir_nftw(path);

    return 0;
}

