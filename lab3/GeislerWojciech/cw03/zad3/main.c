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

// Replaces each space in a string with '\0'
// and returns it as a 2-dimensional array
char **tokenize(char *string) {
    char **result;
    int first = 0;
    // special case for first token
    while(string[first] == ' ') ++first;
    int count = 1;
    int i, j, length;
    for(i = first + 1; string[i] != '\0'; ++i) {
        if(string[i] == ' ') {
            string[i] = '\0';
        } else if (string[i - 1] == '\0'){
            ++count;
        }
    }
    length = i;
    result = calloc((size_t) count + 1, sizeof(char *));
    result[0] = &string[first];
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
    setrlimit(RLIMIT_AS, &r);
    r.rlim_cur = r.rlim_max =  l.time;
    setrlimit(RLIMIT_CPU, &r);
}

// returns exit status of the called process
int run(char** tokens, limits limit){
    int pid = fork();
    if(pid == 0) {
        set_limits(limit);
        execvp(tokens[0], tokens);
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
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

    while(getline(&line, &length, handle) != -1) {
        // remove \n
        line[strlen(line) - 1] = '\0';
        if(run(tokenize(line), limit) != 0){
            printf("Error executing line '%s'\n", line);
            break;
        }
    }

    fclose(handle);
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

