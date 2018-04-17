// Wojciech Geisler 2018

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>

#define MAX_TOKENS 256
const char *const WHITESPACE = " \r\n\t";

typedef struct {
    char *toks[MAX_TOKENS + 1];
    int size;
} tokens;

typedef struct {
    char **commands[MAX_TOKENS];
    int size;
} commands;

// Splits string on whitespace
tokens *tokenize(char *string) {
    tokens *result = calloc(1, sizeof(tokens));
    size_t wordlen;

    // find beginning of string
    string += strspn(string, WHITESPACE);

    while(string != NULL && *string != '\0') {
        wordlen = strcspn(string, WHITESPACE);
        result->toks[result->size] = calloc(1, wordlen);
        strncpy(result->toks[result->size], string, wordlen);
        ++result->size;

        string += wordlen;
        string += strspn(string, WHITESPACE); // skip whitespace
    }
    return result;
}

// Destructively splits command on pipe '|' tokens
// by changing them to null
commands *split_commands(tokens *ts) {
    commands *result = calloc(1, sizeof(commands));
    int i;
    for(i = 0; i < ts->size; ++i) {
        if(ts->toks[i][0] == '|') {
            ts->toks[i] = NULL; // separate commands
        }
        if(i == 0 || ts->toks[i - 1] == NULL) {
            result->commands[result->size++] = &ts->toks[i];
        }
    }
    return result;
}

void run(char **args, int in[2], int out[2]) {
    int pid = fork();
    if(pid == 0) {
        dup2(in[0], STDIN_FILENO);
        dup2(out[1], STDOUT_FILENO);

        close(in[1]);
        close(out[0]);

        execvp(args[0], args);
        exit(126);
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

    int in[2];
    int out[2];

    while(getline(&line, &length, handle) != -1) {
        printf("Executing '%.*s':\n", (int) (strlen(line) - 1), line);
        tokens *ts = tokenize(line);
        commands *c = split_commands(ts);

        in[0] = STDIN_FILENO;
        in[1] = -1;

        for(int i = 0; i < c->size; ++i) {
            if(i == c->size - 1) {
                out[1] = STDOUT_FILENO;
                out[0] = -1;
            } else {
                pipe(out);
            }

            run(c->commands[i], in, out);
            if(in[0] != STDIN_FILENO)
                close(in[0]);
            if(out[1] != STDOUT_FILENO)
                close(out[1]);
            in[0] = out[0];
            in[1] = out[1]; // closed!
        }
        for(int i = 0; i < c->size; ++i) {
            pid_t child = wait(NULL);
            assert(child >= 0);
        }

        printf("\n");
    }

    free(line);
    fclose(handle);
}

int main(int argc, char *argv[]) {
    if(argc < 1) {
        fprintf(stderr, "Batch file is required as the first argument\n");
        exit(1);
    }

    execute_batch(argv[1]);
    return 0;
}

