package main

import "args"
import "core:fmt"
import "core:os"

main :: proc() {

    // The first argument is the binary, so we don't need it.
    argv := os.args[1:];

    // Path to input file (required)
    input_path, ok_arg_input, ok_val_input :=
        args.singleValueOf(argv, { "--input", "-i" });
    if !ok_arg_input || !ok_val_input {
        fmt.println("ERROR: Provide path to input file")
        os.exit(64); // EX_USAGE
    }

    // Path to output file (required)
    output_path, ok_arg_output, ok_val_output :=
        args.singleValueOf(argv, { "--output", "-o" });
    if !ok_arg_output || !ok_val_output {
        fmt.println("ERROR: Provide path to output file")
        os.exit(64); // EX_USAGE
    }


}
