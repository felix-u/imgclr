package main

import "args"
import "core:fmt"
import "core:os"

main :: proc() {

    // The first argument is the binary, so we don't need it.
    argv := os.args[1:];

    fmt.println(args.multipleValuesOf(argv, { "--flag", "-f" }));
}
