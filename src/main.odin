package main

import "args"
import "core:fmt"

main :: proc() {
    fmt.println(args.is_present("something"));
}
