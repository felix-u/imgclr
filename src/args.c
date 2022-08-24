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


// Returns single value of flag

char* args_singleValueOf(int argc, char** argv, char* flag) {

    // Check the flag has actually been passed
    BoolFlagReturn flag_check = args_isPresent(argc, argv, flag);
    if (flag_check.is_present) {

        // If the arg after the flag doesn't start with '-', we return it as
        // the supplied option.
        if (argc > flag_check.index && argv[flag_check.index][0] != '-') {
            return argv[flag_check.index];
        }

        // Otherwise, flag is present but no value supplied
        return "";
    }

    // Flag not present
    return "";
}


// We return an offset and a length for "slicing" argv.

typedef struct MultipleValReturn {
    int offset;
    int length;
} MultipleValReturn;


// Returns multiple values of flag

MultipleValReturn args_multipleValuesOf(int argc, char** argv, char* flag) {

    // Check the flag has actually been passed
    BoolFlagReturn flag_check = args_isPresent(argc, argv, flag);
    if (flag_check.is_present) {

        // If the arg after the flag doesn't start with '-', at least one
        // option was supplied and so there are things to return.
        if (argc > flag_check.index && argv[flag_check.index][0] != '-') {
            int end_index = 0;

            // Stop at first argument which begins with '-'
            for (int i = flag_check.index; i < argc; i++) {
                if (argv[i][0] == '-') {
                    end_index = i;
                }
            }

            // Or, if end_index is still 0, the values go to the end of argv
            if (end_index == 0) end_index = argc;

            MultipleValReturn this_return = {flag_check.index, end_index};
            return this_return;
        }

        // Otherwise, flag is present but no value supplied
        MultipleValReturn this_return = {flag_check.index, 0};
        return this_return;
    }

    // Flag not present
    MultipleValReturn this_return = {0, 0};
    return this_return;
}
