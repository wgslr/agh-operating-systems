#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

typedef struct limits{
    int time;
    int size;
} limits;

void display_usage(char* name, struct rusage *before, struct rusage *after);

// Replaces each space in a string with '\0'
// and returns it as a 2-dimensional array
char **tokenize(char *string) {
    char **result;

    // find beginning of string
    while(string[0] == ' ')
        string = &string[1];

    // special case for first token
    int count = 1;
    int i, j, length;
    for(i = 1; string[i] != '\0'; ++i) {
        if(string[i] == ' ') {
            string[i] = '\0';
        } else if (string[i - 1] == '\0'){
            ++count;
        }
    }
    length = i;
    result = calloc((size_t) count + 1, sizeof(char *));
    result[0] = &string[0];
    j = 1;
    for(i = 1; i < length; ++i) {
        if(string[i] != '\0' && string[i - 1] == '\0') { // do not create tokens from consecutive spaces
            result[j++] = &string[i];
        }
    }
    result[count] = (char*) NULL; // required by execvpe
    return result;
}

void set_limits(limits l){
    struct rlimit r;
    r.rlim_cur = r.rlim_max =  ((rlim_t)l.size * 1024 * 1024);
    printf("Set limit as %zu\n", (rlim_t)l.size * 1024 * 1024);
    setrlimit(RLIMIT_AS, &r);
    r.rlim_cur = r.rlim_max = (rlim_t) l.time;
    setrlimit(RLIMIT_CPU, &r);
}

// returns exit status of the called process
int run(char** tokens, limits limit){
    int pid = fork();
    if(pid == 0) {
        set_limits(limit);
        execvp(tokens[0], tokens);
        exit(126);
    } else {
        int status;
        waitpid(pid, &status, 0);

        if(WIFSIGNALED(status)){
            printf("Process ended because of signal %d\n", WTERMSIG(status));
            return -1;
        }

        return WEXITSTATUS(status);
    }
}

void execute_batch(char *file, limits limit) {
    FILE *handle = fopen(file, "r");
    if(handle == NULL) {
        fprintf(stderr, "Error opening batch file\n");
        exit(1);
    }

    char* line = calloc(255, sizeof(char));
    size_t length = 0;
    struct rusage usage_before;
    struct rusage usage_after;

    while(getline(&line, &length, handle) != -1) {
        // remove \n
        line[strlen(line) - 1] = '\0';

        getrusage(RUSAGE_CHILDREN, &usage_before);

        if(run(tokenize(line), limit) != 0){
            printf("Error executing line '%s'\n", line);
            break;
        } else {
            getrusage(RUSAGE_CHILDREN, &usage_after);
            display_usage(line, &usage_before, &usage_after);
        }
    }

    fclose(handle);
    free(line);
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
        fprintf(stderr, "Expected arguments: <batch_file> <time_limit_seconds> <size_limit_megabytes>\n");
        exit(1);
    }

    limits l;
    l.time = atoi(argv[2]);
    l.size = atoi(argv[3]);

    execute_batch(argv[1], l);

    return 0;
}

void display_usage(char* name, struct rusage *before, struct rusage *after){
    time_t user = (after->ru_utime.tv_sec - before->ru_utime.tv_sec) * 1000000 + after->ru_utime.tv_usec - before->ru_utime.tv_usec;
    time_t system = (after->ru_stime.tv_sec - before->ru_stime.tv_sec) * 1000000 + after->ru_stime.tv_usec - before->ru_stime.tv_usec;
    printf("Finished execution of '%s':\n", name),
    printf("Elapsed user time: %ld us\n", user);
    printf("Elapsed system time: %ld us\n\n", system);
}
