#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>
#include <stdint.h>
#include "args.c"


int main(int argc, char** argv) {

    printf("%d\n", args_isPresent(argc, argv, "-o").is_present);

    return EXIT_SUCCESS;
}
