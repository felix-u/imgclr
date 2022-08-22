package args

import "core:fmt"

// `flag` is an array of two strings: the long flag (e.g. "--option") at
// `flag[0]`, and the short flag (e.g. "-o") at `flag[1]`. We look for the long
// version before the short one.

// `args` is the string array of command-line arguments.


// Checks if a boolean flag exists
isPresent :: proc(args: []string, flag: [2]string) -> (bool, int) {

    // Check for long flag first
    for arg, index in args {
        if arg == flag[0] do return true, index;
    }
    // Then the short flag
    for arg, index in args {
        if arg == flag[1] do return true, index;
    }

    // Flag missing
    return false, 0;
}


// Returns single value of flag
singleValueOf :: proc(args: []string, flag: [2]string) -> (bool, bool, string) {

    // Check that the argument actually exists
    if ok, index := isPresent(args, flag); ok {

        // If the arg after the flag doesn't start with '-', we return it as
        // the supplied option.
        if len(args) > index + 1 && args[index + 1][0] != '-' {
            return true, true, args[index + 1];
        }

        // Otherwise, flag is present but no value supplied
        return true, false, "";
    }

    // Flag not present
    return false, false, "";
}


// Returns multiple values of flag
multipleValuesOf :: proc(args: []string, flag: [2]string) -> (bool, bool, []string) {

    // Check that the argument actually exists
    if ok, index := isPresent(args, flag); ok {

        // If the arg after the flag doesn't start with '-', at least one
        // option was supplied and we return it/them.
        if len(args) > index + 1 && args[index + 1][0] != '-' {
            end_index: int;
            for arg, i in args[index + 1 : ] {
                if arg[0] == '-' {
                    end_index = index + i + 1;
                }
                else if (i + index + 1) == len(args) - 1 {
                    end_index = len(args);
                }
            }

            return true, true, args[index + 1 : end_index];
        }

        // Otherwise, flag is present but no value supplied
        return true, false, {};
    }

    // Flag not present
    return false, false, {};
}
