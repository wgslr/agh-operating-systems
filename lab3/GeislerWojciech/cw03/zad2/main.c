// Wojciech Geisler 2018

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>

// Replaces each space in a string with '\0'
// and returns it as a 2-dimensional array
char **tokenize(char *string) {
    char **result;

    // find beginning of string
    while(string[0] == ' ')
        string = &string[1];

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

// returns exit status of the called process
int run(char** tokens){
    int pid = fork();
    if(pid == 0) {
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

void execute_batch(char *file) {
    FILE *handle = fopen(file, "r");
    if(handle == NULL) {
        fprintf(stderr, "Error opening batch file\n");
        exit(1);
    }

    char* line = NULL;
    size_t length = 0;

    while(getline(&line, &length, handle) != -1) {
        // remove \n
        line[strlen(line) - 1] = '\0';
        printf("Running '%s':\n", line);

        char** args = tokenize(line);

        int result = run(args);
        free(args);

        if(result != 0){
            printf("Job '%s' encountered error\n", line);
            break;
        } else {
            printf("\n");
        }
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
