#include <stdbool.h>
#include <strings.h>
#include <stdio.h>


// We return not just whether the flag is present, but also its index.

typedef struct BoolFlagReturn {
    bool is_present;
    int index;
} BoolFlagReturn;


// Checks if a boolean flag exists

BoolFlagReturn args_isPresent(int argc, char** argv, char** flag) {

    for (int i = 1; i < argc; i++) {
        if (!strcasecmp(argv[i], flag[0])) {
            BoolFlagReturn this_return = {true, i};
            return this_return;
        }
        else if (!strcasecmp(argv[i], flag[1])) {
            BoolFlagReturn this_return = {true, i};
            return this_return;
        }
    }

    // Flag missing
    BoolFlagReturn this_return = {false, 0};
    return this_return;
}


// Returns single value of flag

char* args_singleValueOf(int argc, char** argv, char** flag) {

    // Check the flag has actually been passed
    BoolFlagReturn flag_check = args_isPresent(argc, argv, flag);
    if (flag_check.is_present) {

        // If the arg after the flag doesn't start with '-', we return it as
        // the supplied option.
        if (argc > flag_check.index + 1 && argv[flag_check.index + 1][0] != '-') {
            return argv[flag_check.index + 1];
        }

        // Otherwise, flag is present but no value supplied
        return NULL;
    }

    // Flag not present
    return NULL;
}


// We return an offset and a length for "slicing" argv.

typedef struct MultipleValReturn {
    int offset;
    int end;
} MultipleValReturn;


// Returns multiple values of flag

MultipleValReturn args_multipleValuesOf(int argc, char** argv, char** flag) {

    // Check the flag has actually been passed
    BoolFlagReturn flag_check = args_isPresent(argc, argv, flag);

    if (flag_check.is_present) {

        // If the arg after the flag doesn't start with '-', at least one
        // option was supplied and so there are things to return.
        if (argc > flag_check.index + 1 && argv[flag_check.index + 1][0] != '-') {

            int end_index = 0;

            // Stop at first argument which begins with '-'
            for (int i = flag_check.index + 1; i < argc; i++) {
                if (argv[i][0] == '-') {
                    end_index = i;
                }
            }

            // If end_index is still 0 by now, the values go to the end of argv.
            if (end_index == 0) end_index = argc;

            MultipleValReturn this_return = {flag_check.index + 1, end_index};
            return this_return;
        }

        // Otherwise, flag is present but no value supplied
        MultipleValReturn this_return = {flag_check.index + 1, 0};
        return this_return;
    }

    // Flag not present
    MultipleValReturn this_return = {0, 0};
    return this_return;
}
