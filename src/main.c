#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>
#include <stdint.h>
#include "args.c"


int main(int argc, char** argv) {

    printf("%s\n", args_singleValueOf(argc, argv, "-o"));

    return EXIT_SUCCESS;
}
