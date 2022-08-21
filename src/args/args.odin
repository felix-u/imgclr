package args

// `flag` is an array of two strings: the short flag (e.g. "-o"), and the long
// flag (e.g. "--option"). We look for the long version before the short one.

is_present :: proc(flag: [2]string) -> bool {

    return true;
}
