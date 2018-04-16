// Wojciech Geisler 2018

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKENS 256
const char *const WHITESPACE = " \r\n\t";

typedef struct {
    char *toks[MAX_TOKENS];
    int size;
} tokens;

typedef struct {
    char **commands[MAX_TOKENS];
    int size;
} commands;

// Splits string on whitespace
tokens tokenize(char *string) {
    tokens result = {toks: {0}, size: 0};
//    memset(&result.toks, '\0', MAX_TOKENS * sizeof(char*));
//    result.size = 0;
    int it = 0;
    size_t wordlen;

    // find beginning of string
    while(isspace(string[0]))
        string += 1;

    while(string != NULL && *string != '\0') {
        wordlen = strcspn(string, WHITESPACE);
        result.toks[it] = calloc(1, wordlen);
        strncpy(result.toks[it], string, wordlen);
        ++it;

        string += wordlen;
        string += strspn(string, WHITESPACE); // skip whitespace
    }
    result.size = it;
    return result;
}

// returns exit status of the called process
int run(tokens ts) {
    int pid = fork();
    if(pid == 0) {
        execvp(ts[0], ts);
        exit(126);
    } else {
        int status;
        waitpid(pid, &status, 0);

        if(WIFSIGNALED(status)) {
            printf("Process ended because of signal %d\n", WTERMSIG(status));
            return -1;
        }

        return WEXITSTATUS(status);
    }
}

void execute_batch(char *file) {
    FILE *handle = fopen(file, "r");
    if(handle == NULL) {
        fprintf(stderr, "Error opening batch file\n");
        exit(1);
    }

    char *line = NULL;
    size_t length = 0;

    while(getline(&line, &length, handle) != -1) {
        // remove \n
        line[strlen(line) - 1] = '\0';
        printf("Running '%s':\n", line);

//        char** args = tokenize(line);
//
//        int result = run(args);
//        free(args);
//
//        if(result != 0){
//            printf("Job '%s' encountered error\n", line);
//            break;
//        } else {
//            printf("\n");
//        }
//    }
    }

    free(line);
    fclose(handle);
}

int main(int argc, char *argv[]) {
    if(argc < 1) {
        fprintf(stderr, "Batch file is required as the first argument\n");
        exit(1);
    }

    tokens r;
    char *easy, *str;
    easy = malloc(255);
    str = malloc(255);
    strcpy(easy, "grep def");
    strcpy(str, " Jedne dwa | trzy cztery ");


//    execute_batch(argv[1]);
    return 0;
}

