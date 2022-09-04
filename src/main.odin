package main

import "args"
import "colour"
import "core:fmt"
import "core:os"
import "core:strings"
import image "../libs/wrappers/stb_image"


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


    // Required arguments

    // Path to input file
    input_path, ok_arg_input, ok_val_input :=
        args.singleValueOf(argv, { "--input", "-i" });
    if !ok_arg_input || !ok_val_input {
        fmt.println("ERROR: Provide path to input file");
        os.exit(64); // EX_USAGE
    }

    // Path to output file
    output_path, ok_arg_output, ok_val_output :=
        args.singleValueOf(argv, { "--output", "-o" });
    if !ok_arg_output || !ok_val_output {
        fmt.println("ERROR: Provide path to output file");
        os.exit(64); // EX_USAGE
    }
    _ = output_path;

    // Palette colours
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


    // Optional arguments

    // Dithering algorithm
    dither, ok_arg_dither, ok_val_dither :=
        args.singleValueOf(argv, { "--dither", "-d" });
    if ok_arg_dither && !ok_val_dither {
        fmt.println("ERROR: --dither/-d flag takes one argument");
        os.exit(64); // EX_USAGE
    }
    _ = dither;

    // Swap brightness
    swap, _ := args.isPresent(argv, { "--swap", "-s" });
    _ = swap;

    // Print help
    help, _ := args.isPresent(argv, { "--help", "-h" });
    if help {
        fmt.println(HELP_TEXT);
        os.exit(0);
    }


    // Load image

    input_cstring := strings.clone_to_cstring(input_path, context.temp_allocator);
    width, height, channels : i32;
    data := image.load(input_cstring, &width, &height, &channels, 3);
    defer image.image_free(data);
    if data == nil {
        fmt.println("Image loading failed");
        os.exit(69); // EX_UNAVAILABLE
    }

}
