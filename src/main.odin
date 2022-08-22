package main

import "args"
import "core:fmt"
import "core:os"

main :: proc() {

    // The first argument is the binary, so we don't need it.
    argv := os.args[1:];

    fmt.println("-b/--boolean:", args.isPresent(argv, { "--boolean", "-b" }));
    fmt.println("-s/--single:", args.singleValueOf(argv, { "--single", "-s" }));
    fmt.println("-m/--multiple:", args.multipleValuesOf(argv, { "--multiple", "-m" }));
}
