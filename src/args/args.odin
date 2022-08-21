package args

// `flag` is an array of two strings: the long flag (e.g. "--option") at
// `flag[0]`, and the short flag (e.g. "-o") at `flag[1]`. We look for the long
// version before the short one.

// `args` is the string array of command-line arguments.


// Checks if a boolean argument exists

is_present :: proc(args: []string, flag: [2]string) -> bool {

    // Check for long flag first
    for arg in args {
        if arg == flag[0] do return true;
    }

    // Then the short flag
    for arg in args {
        if arg == flag[1] do return true;
    }

    // If we've not returned true yet, the flag is missing.
    return false;
}
