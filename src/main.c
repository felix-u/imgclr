#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>
#include <stdint.h>


int main(int argc, char** argv) {
    printf("Hej!\n");

    for (int i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }

    return EXIT_SUCCESS;
}
