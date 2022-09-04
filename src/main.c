#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "args.c"
#include "colour.c"
#include "dither.c"

// @Feature stb_image can read PNM but stb_image_write cannot write it. PPM is
// quite simple, so maybe I should implement my own writer. @Feature
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_PNM
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "../libs/stb_image-v2.27/stb_image.h"
#include "../libs/stb_image_write-v1.16/stb_image_write.h"


const char *HELP_TEXT =
"    -h, --help                Display this help information and exit.\n"
"    -i, --input <FILE>        Supply input file.\n"
"    -o, --output <FILE>       Supply output location.\n"
"    -d, --dither <STR>        Specify dithering algorithm, or \"none\" to disable\n"
"                                  (default is \"floyd-steinberg\").\n"
"    -s, --swap                Invert image brightness, preserving hue and\n"
"                              saturation.\n"
"    -p, --palette <STR>...    Supply palette as whitespace-separated colours.\n";


char * extensionFromStr(char *str);

int main(int argc, char **argv) {

    // Help flag

    char **help_arg = (char *[]){"-h", "--help"};
    BoolFlagReturn help = args_isPresent(argc, argv, help_arg);
    if (help.is_present) {
        printf("%s\n", HELP_TEXT);
        return(EXIT_SUCCESS);
    }


    // Required arguments

    char **input_arg = (char *[]){"-i", "--input"};
    char *input_path = args_singleValueOf(argc, argv, input_arg);
    if (input_path == NULL) {
        printf("ERROR: Must provide path to input file.\n");
        exit(EX_USAGE);
    }

    char **output_arg = (char *[]){"-o", "--output"};
    char *output_path = args_singleValueOf(argc, argv, output_arg);
    if (output_path == NULL) {
        printf("ERROR: Must provide output location.\n");
        exit(EX_USAGE);
    }
    char *ext = extensionFromStr(output_path);
    if (ext == NULL) {
        printf("ERROR: Unable to infer output image format.\n");
        exit(EX_USAGE);
    }
    else if (
        strcasecmp(ext, "jpg") && strcasecmp(ext, "jpeg") &&
        strcasecmp(ext, "png") &&
        strcasecmp(ext, "bmp") &&  strcasecmp(ext, "dib"))
    {
        printf("ERROR: Cannot infer image format from extension ");
        printf("\"%s\".\n", ext);
        exit(EX_USAGE);
    }

    char **palette_arg = (char *[]){"-p", "--palette"};
    MultipleValReturn palette_return =
        args_multipleValuesOf(argc, argv, palette_arg);
    if (palette_return.offset == 0 || palette_return.end == 0 ||
        palette_return.end - palette_return.offset < 2)
    {
        printf("ERROR: Must provide at least two (2) palette colours.\n");
        exit(EX_USAGE);
    }


    // Optional arguments

    char **dither_arg = (char *[]){"-d", "--dither"};
    BoolFlagReturn dither_is_present = args_isPresent(argc, argv, dither_arg);
    char *dither_alg = NULL;

    // Algorithm is floyd-steinberg unless user specifies otherwise
    Algorithm algorithm = floyd_steinberg;

    if (dither_is_present.is_present) {
        dither_alg = args_singleValueOf(argc, argv, dither_arg);
        if (dither_alg == NULL) {
            printf("ERROR: `dither` flag requires argument.\n");
            exit(EX_USAGE);
        }
        else {
            bool found_algorithm = false;
            for (int i = 0; i < NUM_OF_ALGORITHMS; i++) {
                if (!strcasecmp(dither_alg, ALGORITHMS[i]->name)) {
                    algorithm = *ALGORITHMS[i];
                    found_algorithm = true;
                    break;
                }
            }

            if (!found_algorithm) {
                printf("ERROR: Unsupported algorithm \"%s\".\n", dither_alg);
                exit(EX_USAGE);
            }

        }
    }


    char **swap_arg = (char *[]){"-s", "--swap"};
    BoolFlagReturn swap = args_isPresent(argc, argv, swap_arg);


    // Convert palette hex strings to an array of RGB

    int palette_len = palette_return.end - palette_return.offset;
    RGB palette[palette_len];

    for (int i = 0; i < palette_len; i++) {
        RGBCheck rgb_get = hexStrToRGB(argv[i + palette_return.offset]);
        if (rgb_get.valid == true) {
            RGB rgb_put = {rgb_get.r, rgb_get.g, rgb_get.b};
            palette[i] = rgb_put;
        }
        else {
            printf("ERROR: \"%s\" is not a valid hex colour.\n",
                   argv[i + palette_return.offset]);
            exit(EX_USAGE);
        }
    }


    // Load image and do cool stuff

    int width = 0, height = 0, channels = 0;
    unsigned char *data = stbi_load(input_path, &width, &height, &channels, 3);
    printf("%d x %d with %d channels\n", width, height, channels);
    if (data == NULL) {
        const char *reason = stbi_failure_reason();
        printf("ERROR: Could not load \"%s\":\n%s\n", input_path, reason);
        exit(EX_NOINPUT);
    }

    // @Missing Dithering @Missing

    // Convert image to palette

    int data_len = width * height * channels;

    for (int i = 0; i < data_len; i += 3) {

        int min_diff = 999;
        int best_match;
        for (int j = 0; j < palette_len; j++) {
            int diff_r = abs(data[i + 0] - palette[j].r);
            int diff_g = abs(data[i + 1] - palette[j].g);
            int diff_b = abs(data[i + 2] - palette[j].b);
            int diff_total = diff_r + diff_g + diff_b;
            if (diff_total < min_diff) {
                min_diff = diff_total;
                best_match = j;
            }
        }

        data[i + 0] = palette[best_match].r;
        data[i + 1] = palette[best_match].g;
        data[i + 2] = palette[best_match].b;
    }


    // Write to output path, with correct format based on extension

    int write_success = false;
    if (!strcasecmp(ext, "jpg") || !strcasecmp(ext, "jpeg")) {
        write_success = stbi_write_jpg(output_path, width, height, channels, data, 100);
    }
    else if (!strcasecmp(ext, "png")) {
        int stride_in_bytes = width * channels;
        write_success = stbi_write_png(output_path, width, height, channels, data, stride_in_bytes);
    }
    else if (!strcasecmp(ext, "bmp") || !strcasecmp(ext, "dib")) {
        write_success = stbi_write_bmp(output_path, width, height, channels, data);
    }

    if (write_success) {
        printf("Wrote image of size %dx%d to %s.\n",
               width, height, output_path);
    }
    else {
        printf("ERROR: Unable to write image to %s.\n", output_path);
        exit(EX_UNAVAILABLE);
    }

    stbi_image_free(data);
    return EXIT_SUCCESS;
}


// Get file extension from string

char * extensionFromStr(char *str) {

    int str_len = strlen(str);
    int ext_pos = 0;

    for (int i = 0; i < str_len; i++) {
        if (str[i] == '.') {
            ext_pos = i;
            break;
        }
    }

    if (ext_pos != 0 && ext_pos < str_len - 1) {
        return str + ext_pos + 1;
    }

    return NULL;
}
