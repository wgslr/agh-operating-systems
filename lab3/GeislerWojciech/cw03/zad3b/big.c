// Wojciech Geisler
// Program allocating memory blocks of given size one by one.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if(argc < 3) {
        fprintf(stderr, "Invocation: <blocks_count> <blocks_size>\n");
    }

    size_t blocks = strtoul(argv[1], NULL, 10);
    size_t size = strtoul(argv[2], NULL, 10);
    size_t total = 0;
    char *block;
    for(unsigned i = 0; i < blocks; ++i) {
        printf("Allocating %zu MiB for a total of %zu MiB\n", size, total + size);
        block = malloc(size * 1024 * 1024);
        memset(block, 1, size * 1024 * 1024);
        total += size;
    }

    return 0;
}

