#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <stdbool.h>
#include <string.h>

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

// returns exit status of the called process
int run(char** tokens){
    int pid = fork();
    if(pid == 0) {
        execvp(tokens[0], tokens);
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

void execute_batch(char *file) {
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
        fprintf(stderr, "Read line '%s'\n", line);
        if(run(tokenize(line)) != 0){
            printf("Error executing line '%s'\n", line);
            break;
        }
    }

    fclose(handle);
}

int main(int argc, char *argv[]) {
    if(argc < 1) {
        fprintf(stderr, "Batch file is required as the first argument\n");
    }

    execute_batch(argv[1]);
}
