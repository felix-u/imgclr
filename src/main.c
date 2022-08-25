#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "args.c"
#define STB_DS_IMPLEMENTATION
#include "../libs/stb_ds-v0.67/stb_ds.h"


int main(int argc, char** argv) {

    char *input_arg;
    if (args_isPresent(argc, argv, "-i").is_present == true) {
        input_arg = "-i";
    }
    else {
        printf("ERROR: Provide path to input file\n");
        exit(EX_USAGE);
    }
    printf("%s\n", input_arg);

    return EXIT_SUCCESS;
}
