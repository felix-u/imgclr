package main

import "args"
import "colour"
import "core:fmt"
import "core:os"

HELP_TEXT ::
`    -h, --help                Display this help information and exit.
    -i, --input <FILE>        Supply input file.
    -o, --output <FILE>       Supply output location.
    -d, --dither <STR>        Specify dithering algorithm, or "none" to disable
                                  (default is "floyd-steinberg").
    -s, --swap                Invert image brightness, preserving hue and
                              saturation.
    -p, --palette <STR>...    Supply palette as whitespace-separated colours.
`


main :: proc() {

    // The first argument is the binary, so we don't need it.
    argv := os.args[1 : ];

    // Path to input file (required)
    input_path, ok_arg_input, ok_val_input :=
        args.singleValueOf(argv, { "--input", "-i" });
    if !ok_arg_input || !ok_val_input {
        fmt.println("ERROR: Provide path to input file");
        os.exit(64); // EX_USAGE
    }

    // Path to output file (required)
    output_path, ok_arg_output, ok_val_output :=
        args.singleValueOf(argv, { "--output", "-o" });
    if !ok_arg_output || !ok_val_output {
        fmt.println("ERROR: Provide path to output file");
        os.exit(64); // EX_USAGE
    }

    // Palette colours (required)
    palette_input, ok_arg_palette, ok_val_palette :=
        args.multipleValuesOf(argv, { "--palette", "-p" });
    if !ok_arg_palette || !ok_val_palette {
        fmt.println("ERROR: Provide palette colours");
        os.exit(64); // EX_USAGE
    }
    if len(palette_input) < 2 {
        fmt.println("ERROR: Must supply at least two (2) colours");
        os.exit(64); // EX_USAGE
    }

    palette: [dynamic]colour.RGBA;
    for str in palette_input {
        if clr, ok := colour.hexStrToRGBA(str); ok {
            append(&palette, clr);
        }
    }

    // Dithering algorithm (optional)
    dither, ok_arg_dither, ok_val_dither :=
        args.singleValueOf(argv, { "--dither", "-d" });
    if ok_arg_dither && !ok_val_dither {
        fmt.println("ERROR: --dither/-d flag takes one argument");
        os.exit(64); // EX_USAGE
    }

    // Swap brightness (optional)
    swap, _ := args.isPresent(argv, { "--swap", "-s" });

    // Print help (optional)
    help, _ := args.isPresent(argv, { "--help", "-h" });
    if help {
        fmt.println(HELP_TEXT);
        os.exit(0);
    }



}
