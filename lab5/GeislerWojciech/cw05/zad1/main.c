// Wojciech Geisler 2018

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>

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
tokens tokenize(char *string) {
    tokens result = { .toks = {0}, .size = 0};
//    memset(&result.toks, '\0', MAX_TOKENS * sizeof(char*));
//    result.size = 0;
    size_t wordlen;

    // find beginning of string
    while(isspace(string[0]))
        string += 1;

    while(string != NULL && *string != '\0') {
        wordlen = strcspn(string, WHITESPACE);
        result.toks[result.size] = calloc(1, wordlen);
        strncpy(result.toks[result.size], string, wordlen);
        ++result.size;

        string += wordlen;
        string += strspn(string, WHITESPACE); // skip whresult.sizeespace
    }
    return result;
}

commands split_commands(tokens ts) {
    commands result = {.size = 0, .commands = {0}};
    int pos = 0;
}

// returns exit status of the called process
void run(tokens ts, int in, int out, int close1, int close2) {
    int pid = fork();
    if(pid == 0) {
        dup2(in, STDIN_FILENO);
        dup2(out, STDOUT_FILENO);

        if(close1 >= 0){
            close(close1);
        }
        if(close2 >= 0){
            close(close2);
        }

        execvp(ts.toks[0], ts.toks);
        exit(126);
    } else {
        printf("Spawned %d\n", pid);
//        int status;
//        waitpid(pid, &status, 0);
//
//        if(WIFSIGNALED(status)) {
//            printf("Process ended because of signal %d\n", WTERMSIG(status));
//            return -1;
//        }
//
//        return WEXITSTATUS(status);
    }
}
//
//void execute_batch(char *file) {
//    FILE *handle = fopen(file, "r");
//    if(handle == NULL) {
//        fprintf(stderr, "Error opening batch file\n");
//        exit(1);
//    }
//
//    char *line = NULL;
//    size_t length = 0;
//
//    while(getline(&line, &length, handle) != -1) {
//        // remove \n
//        line[strlen(line) - 1] = '\0';
//        printf("Running '%s':\n", line);

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
//    }
//
//    free(line);
//    fclose(handle);
//}

int main(int argc, char *argv[]) {
    if(argc < 1) {
        fprintf(stderr, "Batch file is required as the first argument\n");
        exit(1);
    }

    char *uname, *sed, *cat;
    uname = malloc(255);
    sed = malloc(255);
    cat = malloc(255);
    strcpy(uname, "uname -snm");
    strcpy(sed, "sed s/\\s/\\n/g");
    strcpy(cat, "cat -n");

    tokens t1 = tokenize(uname);
    tokens t2 = tokenize(sed);
    tokens t3 = tokenize(cat);

    int fd[2];
    int fd2[2];
    pipe(fd);
    pipe(fd2);
    run(t1, fd[0], fd[1], fd[0], -1);
//    close(fd[0]);
    close(fd[1]);
//    run(t2, fd[0], STDOUT_FILENO, fd[1], fd2[0]);
    run(t2, fd[0], fd2[1], fd[1], -1);
    close(fd[0]);
    run(t3, fd2[0], fd2[1], -1, -1);
    close(fd2[1]);


//    dup2(STDOUT_FILENO, fd[0]);
//    dup2(fd[0], STDOUT_FILENO);
//    sleep(1);
    char* buff = calloc(100, sizeof(char));
    read(fd2[0], buff, 99);
    printf("Read from pipe2: '%s'\n", buff);


    close(fd[0]);
    close(fd[1]);
    close(fd2[0]);
    close(fd2[1]);

//    run(t2, fd[1], STDOUT_FILENO);
//    close(fd[0]);

    printf("Wait1\n");
    pid_t trup = wait(NULL);
    printf("Waited %d\n", trup);
    printf("Wait2\n");
    trup = wait(NULL);
    printf("Waited %d\n", trup);
    printf("Wait3\n");
    trup = wait(NULL);
    printf("Waited %d\n", trup);


//    execute_batch(argv[1]);
    return 0;
}

