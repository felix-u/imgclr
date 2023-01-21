#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define ARGS_IMPLEMENTATION
#define ARGS_BINARY_NAME "imgclr"
#define ARGS_BINARY_VERSION "0.1-dev"
#include "args.h"
#include "int_types.h"

#include "colour.c"
#include "dither.c"

/* @Feature { stb_image can read PNM but stb_image_write cannot write it. PPM is
              quite simple, so maybe I should implement my own writer.
} */
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_PNM
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "../libs/stb_image-v2.27/stb_image.h"
#include "../libs/stb_image_write-v1.16/stb_image_write.h"


#define EX_USAGE 64
#define EX_NOINPUT 66
#define EX_UNAVAILABLE 69


char * extensionFromStr(char *str);


int main(int argc, char **argv) {

    args_Flag dither_flag = {
        'd', "dither",
        "specify dithering algorithm, or \"none\" to disable. Default\n"
            "is \"floyd-steinberg\"",
        ARGS_OPTIONAL,
        false, NULL, 0,
        ARGS_SINGLE_OPT, ARGS_EXPECTS_STRING
    };
    args_Flag palette_flag = {
        'p', "palette",
        "supply palette as whitespace-separated hex colours",
        ARGS_REQUIRED,
        false, NULL, 0,
        ARGS_MULTI_OPT, ARGS_EXPECTS_STRING
    };
    args_Flag swap_flag = {
        's', "swap",
        "invert image brightness, preserving hue and saturation",
        ARGS_OPTIONAL,
        false, NULL, 0,
        ARGS_BOOLEAN, ARGS_EXPECTS_NONE
    };

    args_Flag *flags[] = {
        &dither_flag,
        &palette_flag,
        &swap_flag,
        &ARGS_HELP_FLAG,
        &ARGS_VERSION_FLAG,
    };

    const usize flags_count = sizeof(flags) / sizeof(flags[0]);
    usize positional_num = 0;
    const usize positional_cap = 256;
    char *positional_args[positional_cap];
    int args_return = args_process(argc, argv, "image colouriser", flags_count, flags,
                                   &positional_num, positional_args, ARGS_EXPECTS_FILE, ARGS_POSITIONAL_MULTI,
                                   positional_cap);
    if (args_return != ARGS_RETURN_CONTINUE) return args_return;

    if (positional_num < 2) {
        printf("%s: expected output file as positional argument\n", ARGS_BINARY_NAME);
        args_helpHint();
        return EX_USAGE;
    }


    char *input_path = positional_args[0];
    char *output_path = positional_args[1];
    char *ext = extensionFromStr(output_path);
    if (ext == NULL) {
        printf("%s: unable to infer output image format\n", ARGS_BINARY_NAME);
        args_helpHint();
        return EX_USAGE;
    }

    if (
        strcasecmp(ext, "jpg") && strcasecmp(ext, "jpeg") &&
        strcasecmp(ext, "png") &&
        strcasecmp(ext, "bmp") &&  strcasecmp(ext, "dib"))
    {
        printf("%s: cannot infer image format from extension '%s'\n", ARGS_BINARY_NAME, ext);
        args_helpHint();
        return EX_USAGE;
    }

    if (palette_flag.opts_num < 2) {
        printf("%s: must provide at least two (2) palette colours\n", ARGS_BINARY_NAME);
        args_helpHint();
        return EX_USAGE;
    }


    const Algorithm *algorithm = &floyd_steinberg;
    char *dither_alg = dither_flag.is_present ? dither_flag.opts[0] : "floyd-steinberg";
    bool found_algorithm = false;
    for (usize i = 0; i < NUM_OF_ALGORITHMS; i++) {
        if (!strncasecmp(dither_alg, ALGORITHMS[i]->name, strlen(dither_alg))) {
            found_algorithm = true;
            algorithm = ALGORITHMS[i];
            break;
        }
    }

    if (!found_algorithm) {
        printf("%s: invalid dithering algorithm '%s'\n", ARGS_BINARY_NAME, dither_alg);
        return EX_USAGE;
    }


    // Convert palette hex strings to an array of RGB

    RGB palette[palette_flag.opts_num];

    for (int i = 0; i < palette_flag.opts_num; i++) {
        RGBCheck rgb_get = hexStrToRGB(palette_flag.opts[i]);
        if (!rgb_get.valid) {
            printf("%s: '%s' is not a valid hex colour\n", ARGS_BINARY_VERSION, palette_flag.opts[i]);
            args_helpHint();
            return EX_USAGE;
        }
        RGB rgb_put = {rgb_get.r, rgb_get.g, rgb_get.b};
        palette[i] = rgb_put;
    }


    // Load image

    int width = 0, height = 0, channels = 0;
    unsigned char *data = stbi_load(input_path, &width, &height, &channels, 3);
    if (data == NULL) {
        const char *reason = stbi_failure_reason();
        printf("ERROR: Could not load \"%s\":\n%s\n", input_path, reason);
        return EX_NOINPUT;
    }

    int data_len = width * height * channels;


    // Invert brightness, if applicable
    if (swap_flag.is_present) {
        for (int i = 0; i < data_len; i += 3) {

            int brightness = (data[i + 0] + data[i + 1] + data[i + 2]) / 3;
            int r_relative = data[i + 0] - brightness;
            int g_relative = data[i + 1] - brightness;
            int b_relative = data[i + 2] - brightness;

            int new_r = (255 - brightness) + r_relative;
            int new_g = (255 - brightness) + g_relative;
            int new_b = (255 - brightness) + b_relative;

            // Clamp to 0 - 255
            if (new_r < 0) new_r = 0; else if (new_r > 255) new_r = 255;
            if (new_g < 0) new_g = 0; else if (new_g > 255) new_g = 255;
            if (new_b < 0) new_b = 0; else if (new_b > 255) new_b = 255;

            data[i + 0] = (char)new_r;
            data[i + 1] = (char)new_g;
            data[i + 2] = (char)new_b;
        }
    }

    // NOTE: I no longer think this is worth the effort, since a few int
    // comparisons per pixel don't seem necessarily more expensive than a few
    // additions and divisions to calculate the target index from an x and y
    // coordinate. It might even be cheaper the way it's done now. But I'm
    // leaving this here in case I want to come back to it.

    // // Establish bounds for edge-case control loops, when not all error can be
    // // diffused because some of the neighbouring pixels targeted by the
    // // dithering algorithm are out of bounds.
    //
    // int x_bound_left = 0;
    // int x_bound_right = width;
    // int y_bound_bottom = height;
    //
    // for (int i = 0; i < algorithm->offset_num; i++) {
    //     if (x_bound_left + algorithm->offsets[i].x < 0) {
    //         x_bound_left = 0 - algorithm->offsets[i].x;
    //     }
    //     if (width - algorithm->offsets[i].x < x_bound_right) {
    //         x_bound_right = width - algorithm->offsets[i].x;
    //     }
    //     if (height - algorithm->offsets[i].y < y_bound_bottom) {
    //         y_bound_bottom = height - algorithm->offsets[i].y;
    //     }
    // }


    // Convert image to palette

    for (int i = 0; i < data_len; i += 3) {

        int min_diff = 999;
        int best_match;
        for (int j = 0; j < palette_flag.opts_num; j++) {
            int diff_r = abs(data[i + 0] - palette[j].r);
            int diff_g = abs(data[i + 1] - palette[j].g);
            int diff_b = abs(data[i + 2] - palette[j].b);
            int diff_total = diff_r + diff_g + diff_b;
            if (diff_total < min_diff) {
                min_diff = diff_total;
                best_match = j;
            }
        }

        int quant_error[3] = {
            (int)data[i + 0] - palette[best_match].r,
            (int)data[i + 1] - palette[best_match].g,
            (int)data[i + 2] - palette[best_match].b
        };

        // Set current pixel to best match

        data[i + 0] = palette[best_match].r;
        data[i + 1] = palette[best_match].g;
        data[i + 2] = palette[best_match].b;

        // Compute and apply quantisation error

        int current_x = (i / channels) % width;
        int current_y = (i / channels) / width;

        for (int j = 0; j < algorithm->offset_num; j++) {

            int target_x = current_x + algorithm->offsets[j].x;
            int target_y = current_y + algorithm->offsets[j].y;

            // @Speed We have the information to avoid doing bounds checking
            // on the majority of the image, so this is low-hanging fruit. But
            // this is probably not worth it (see NOTE above). @Speed

            if (target_x >= 0 && target_x < width &&
                target_y >= 0 && target_y < height)
            {
                int target_index = channels * (target_y * width + target_x);
                int new_r =
                    (int)data[target_index + 0] +
                    (float)quant_error[0] * algorithm->offsets[j].ratio;
                int new_g =
                    (int)data[target_index + 1] +
                    (float)quant_error[1] * algorithm->offsets[j].ratio;
                int new_b =
                    (int)data[target_index + 2] +
                    (float)quant_error[2] * algorithm->offsets[j].ratio;

                // Clamp to 0 - 255
                if (new_r < 0) new_r = 0; else if (new_r > 255) new_r = 255;
                if (new_g < 0) new_g = 0; else if (new_g > 255) new_g = 255;
                if (new_b < 0) new_b = 0; else if (new_b > 255) new_b = 255;

                data[target_index + 0] = (char)new_r;
                data[target_index + 1] = (char)new_g;
                data[target_index + 2] = (char)new_b;

            }

        }

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
        return EX_UNAVAILABLE;
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
