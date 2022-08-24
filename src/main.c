#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>
#include <stdint.h>
#include "args.c"


int main(int argc, char** argv) {

    volatile MultipleValReturn flag_check = args_multipleValuesOf(argc, argv, "-o");
    for (int i = flag_check.offset; i < flag_check.length - 2; i++) {
        printf("%s\n", argv[i]);
    }

    // printf("%s\n", args_singleValueOf(argc, argv, "-o"));

    return EXIT_SUCCESS;
}
