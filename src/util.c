#include <stdio.h>
#include <stdlib.h>

void error(char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
