#include <stdbool.h>
#include <string.h>
#include <stdio.h>


// We return not just whether the flag is present, but also its index.

typedef struct BoolFlagReturn {
    bool is_present;
    int index;
} BoolFlagReturn;


// Checks if a boolean flag exists

BoolFlagReturn args_isPresent(int argc, char** argv, char* flag) {

    // Look for long flag first
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], flag)) {
            BoolFlagReturn this_return = {true, i};
            return this_return;
        }
    }

    // Flag missing
    BoolFlagReturn this_return = {false, 0};
    return this_return;
}


// // Returns single value of flag
// char* args_singleValueOf(int argc, char** argv, char* flag[2]) {
//
//     // Check the flag has actually been passed
//     if (isPresent(argc, argv, flag).is_present) {
//
//         // If the arg after the flag doesn't start with '-', we return it as
//         // the supplied option.
//         if (argc > )
//     }
// }
