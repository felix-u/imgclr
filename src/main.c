#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define ARGS_IMPLEMENTATION
#define ARGS_BINARY_NAME "imgclr"
#define ARGS_BINARY_VERSION "0.2-dev"
#include "args.h"
#define CLR_IMPLEMENTATION
#include "clr.h"
#include "int_types.h"
#define DITHER_IMPLEMENTATION
#include "dither.h"

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


char *extensionFromStr(char *str);


int main(int argc, char **argv) {

    args_Flag dither_flag = {
        .name_short = 'd',
        .name_long = "dither",
        .help_text = "specify dithering algorithm, or 'none' to disable. Default\n"
                     "is 'floyd-steinberg'. Other options are: 'atkinson', 'jjn',\n"
                     "'burkes', and 'sierra-lite'",
        .required = false,
        .type = ARGS_SINGLE_OPT,
        .expects = ARGS_EXPECTS_STRING,
    };
    args_Flag palette_flag = {
        .name_short = 'p',
        .name_long = "palette",
        .help_text = "supply palette as whitespace-separated hex colours",
        .required = true,
        .type = ARGS_MULTI_OPT,
        .expects = ARGS_EXPECTS_STRING,
    };
    args_Flag swap_flag = {
        .name_short = 's',
        .name_long = "swap",
        .help_text = "invert image brightness, preserving hue and saturation",
        .required = false,
        .type = ARGS_BOOLEAN,
        .expects = ARGS_EXPECTS_NONE,
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

    if (strcasecmp(ext, "jpg") && strcasecmp(ext, "jpeg") &&
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


    const dither_Algorithm *algorithm = &floyd_steinberg;
    char *dither_alg = dither_flag.is_present ? dither_flag.opts[0] : "floyd-steinberg";
    bool found_algorithm = false;
    for (usize i = 0; i < DITHER_ALGORITHM_NUM; i++) {
        if (!strncasecmp(dither_alg, DITHER_ALGORITHMS[i]->name, strlen(dither_alg))) {
            found_algorithm = true;
            algorithm = DITHER_ALGORITHMS[i];
            break;
        }
    }

    if (!found_algorithm) {
        printf("%s: invalid dithering algorithm '%s'\n", ARGS_BINARY_NAME, dither_alg);
        return EX_USAGE;
    }


    // Convert palette hex strings to an array of RGB

    clr_RGB palette[palette_flag.opts_num];

    for (usize i = 0; i < palette_flag.opts_num; i++) {
        clr_RGB *rgb_get = &(clr_RGB){ 0, 0, 0 };
        rgb_get = clr_hexToRGB(palette_flag.opts[i], rgb_get);
        if (rgb_get == NULL) {
            printf("%s: '%s' is not a valid hex colour\n", ARGS_BINARY_NAME, palette_flag.opts[i]);
            args_helpHint();
            return EX_USAGE;
        }
        clr_RGB rgb_put = {rgb_get->r, rgb_get->g, rgb_get->b};
        palette[i] = rgb_put;
    }


    // Load image

    int width = 0, height = 0, channels = 0;
    unsigned char *data = stbi_load(input_path, &width, &height, &channels, 3);
    if (data == NULL) {
        const char *reason = stbi_failure_reason();
        printf("%s: could not load '%s':\n%s\n", ARGS_BINARY_NAME, input_path, reason);
        return EX_NOINPUT;
    }

    usize data_len = width * height * channels;


    // Invert brightness, if applicable
    if (swap_flag.is_present) {
        for (usize i = 0; i < data_len; i += 3) {

            i16 brightness = (data[i + 0] + data[i + 1] + data[i + 2]) / 3;
            i16 r_relative = data[i + 0] - brightness;
            i16 g_relative = data[i + 1] - brightness;
            i16 b_relative = data[i + 2] - brightness;

            i16 new_r = (255 - brightness) + r_relative;
            i16 new_g = (255 - brightness) + g_relative;
            i16 new_b = (255 - brightness) + b_relative;

            // Clamp to 0 - 255
            if (new_r < 0) new_r = 0; else if (new_r > 255) new_r = 255;
            if (new_g < 0) new_g = 0; else if (new_g > 255) new_g = 255;
            if (new_b < 0) new_b = 0; else if (new_b > 255) new_b = 255;

            data[i + 0] = (u8)new_r;
            data[i + 1] = (u8)new_g;
            data[i + 2] = (u8)new_b;
        }
    }

    // NOTE: Having several loops to avoid bounds checking on the majority of the image may seem appealing. It is NOT
    // worth it.

    for (usize i = 0; i < data_len; i += channels) {

        u16 min_diff = 999;
        usize best_match = 0;
        for (usize j = 0; j < palette_flag.opts_num; j++) {
            u16 diff_r = abs(data[i + 0] - palette[j].r);
            u16 diff_g = abs(data[i + 1] - palette[j].g);
            u16 diff_b = abs(data[i + 2] - palette[j].b);
            u16 diff_total = diff_r + diff_g + diff_b;
            if (diff_total < min_diff) {
                min_diff = diff_total;
                best_match = j;
            }
        }

        i16 quant_err[3] = {
            (i16)data[i + 0] - palette[best_match].r,
            (i16)data[i + 1] - palette[best_match].g,
            (i16)data[i + 2] - palette[best_match].b
        };

        // Set current pixel to best match

        data[i + 0] = palette[best_match].r;
        data[i + 1] = palette[best_match].g;
        data[i + 2] = palette[best_match].b;

        // Compute and apply quantisation error

        usize current_x = (i / channels) % width;
        usize current_y = (i / channels) / width;

        for (usize j = 0; j < algorithm->offset_num; j++) {

            isize target_x = current_x + algorithm->offsets[j].x;
            isize target_y = current_y + algorithm->offsets[j].y;

            if (target_x < 0 || target_x >= width || target_y < 0 || target_y >= height) continue;

            usize target_i = channels * (target_y * width + target_x);
            i16 new_r = (i16)data[target_i + 0] + (double)quant_err[0] * algorithm->offsets[j].ratio;
            i16 new_g = (i16)data[target_i + 1] + (double)quant_err[1] * algorithm->offsets[j].ratio;
            i16 new_b = (i16)data[target_i + 2] + (double)quant_err[2] * algorithm->offsets[j].ratio;

            // Clamp to 0 - 255
            if (new_r < 0) new_r = 0; else if (new_r > 255) new_r = 255;
            if (new_g < 0) new_g = 0; else if (new_g > 255) new_g = 255;
            if (new_b < 0) new_b = 0; else if (new_b > 255) new_b = 255;

            data[target_i + 0] = (u8)new_r;
            data[target_i + 1] = (u8)new_g;
            data[target_i + 2] = (u8)new_b;

        }

    }

    // Write to output path, with correct format based on extension

    bool write_success = false;
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

    if (!write_success) {
        printf("%s: unable to write image to '%s'\n", ARGS_BINARY_NAME, output_path);
        stbi_image_free(data);
        return EX_UNAVAILABLE;
    }

    printf("%s: wrote image of size %dx%d to '%s'\n", ARGS_BINARY_NAME, width, height, output_path);

    stbi_image_free(data);
    return EXIT_SUCCESS;
}


// Get file extension from string
char *extensionFromStr(char *str) {
    usize str_len = strlen(str);
    usize ext_pos = 0;

    for (usize i = 0; i < str_len; i++) {
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
