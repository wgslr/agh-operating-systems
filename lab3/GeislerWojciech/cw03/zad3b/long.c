// Wojciech Geisler 2018
// Program executing trygonometric operations n thousand times

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if(argc < 1) {
        fprintf(stderr, "Invocation: <thousands_of_operations>\n");
    }

    double x = 253;
    double y = -223;

    long operations = strtol(argv[1], NULL, 10);
    for(int i = 0; i < operations; ++i) {
        for(int j = 0; j < 1000; ++j) {
            x = sin(y);
            y = sin(x);
        }
    }
    printf("After %lu thousand operations result is %lf\n", operations, x);
    return 0;
}



