#include "base.c"

#define ARGS_IMPLEMENTATION
#define ARGS_BINARY_NAME "imgclr"
#define ARGS_BINARY_VERSION "0.2-dev"
#include "args.h"

#include "colour.c"
#include "dither.c"

#ifndef DEBUG
    #include "stbi.c"
#else
    #include "stb_image.h"
    #include "stb_image_write.h"
#endif // DEBUG

static error extension_from_cstr(char *cstr, char **ext) {
    usize str_len = strlen(cstr);
    usize ext_pos = str_len;
    for (usize i = str_len; i >= 0; i--) {
        if (cstr[i] != '.') continue;
        ext_pos = i;
        break;
    }
    if (ext_pos + 1 >= str_len) {
        return err("unable to infer output image format");
    }
    *ext = cstr + ext_pos + 1;
    return 0;
}

static error main_wrapper(int argc, char **argv) {
    args_Flag dither_flag = {
        .name_short = 'd',
        .name_long = "dither",
        .help_text = "specify dithering algorithm, or 'none' to disable.\n"
                     "Default is 'floyd-steinberg'. Other options are: \n"
                     "'atkinson', 'jjn', 'burkes', and 'sierra-lite'",
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
    int args_return = args_process(
        argc, 
        argv, 
        "image colouriser", 
        flags_count, 
        flags,
        &positional_num, 
        positional_args, 
        ARGS_EXPECTS_FILE, 
        ARGS_POSITIONAL_MULTI,
        positional_cap
    );
    if (args_return != ARGS_RETURN_CONTINUE) return args_return;

    if (positional_num < 2) {
        error e = err("expected output file as positional argument");
        args_helpHint();
        return e;
    }

    char *input_path = positional_args[0];
    char *output_path = positional_args[1];
    char *ext; try (extension_from_cstr(output_path, &ext));

    if (strcasecmp(ext, "jpg") && strcasecmp(ext, "jpeg") &&
        strcasecmp(ext, "png") &&
        strcasecmp(ext, "bmp") && strcasecmp(ext, "dib"))
    {
        error e = errf("cannot infer image format from extension '%s'", ext);
        args_helpHint();
        return e;
    }

    if (palette_flag.opts_num < 2) {
        error e = err("must provide at least two (2) palette colours");
        args_helpHint();
        return e;
    }

    const Dither_Algorithm *algorithm = &floyd_steinberg;
    char *dither_alg = dither_flag.is_present 
        ? dither_flag.opts[0] 
        : "floyd-steinberg";
    bool found_algorithm = false;
    for (usize i = 0; i < DITHER_ALGORITHM_NUM; i++) {
        if (strncasecmp(
                dither_alg, 
                DITHER_ALGORITHMS[i]->name, 
                strlen(dither_alg)
        )) {
            continue;
        }
        found_algorithm = true;
        algorithm = DITHER_ALGORITHMS[i];
        break;
    }
    if (!found_algorithm) {
        return errf("invalid dithering algorithm '%s'", dither_alg);
    }


    // Convert palette hex strings to an array of RGB

    Rgb palette[palette_flag.opts_num];

    for (usize i = 0; i < palette_flag.opts_num; i++) {
        Rgb *rgb_get = &(Rgb){ 0, 0, 0 };
        rgb_get = clr_hexToRGB(palette_flag.opts[i], rgb_get);
        if (rgb_get == NULL) {
            printf("%s: '%s' is not a valid hex colour\n", ARGS_BINARY_NAME, palette_flag.opts[i]);
            args_helpHint();
            return 1;
        }
        Rgb rgb_put = {rgb_get->r, rgb_get->g, rgb_get->b};
        palette[i] = rgb_put;
    }


    // Load image

    int width = 0, height = 0, channels = 0;
    unsigned char *data = stbi_load(input_path, &width, &height, &channels, 3);
    if (data == NULL) {
        const char *reason = stbi_failure_reason();
        printf("%s: could not load '%s':\n%s\n", ARGS_BINARY_NAME, input_path, reason);
        return 1;
    }

    usize data_len = width * height * channels;

    // Invert brightness, if applicable
    if (swap_flag.is_present) for (usize i = 0; i < data_len; i += 3) {
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

    // NOTE: Having several loops to avoid bounds checking on the majority of the image is not worth it.

    for (usize i = 0; i < data_len; i += channels) {

        u16 min_diff = 999;
        usize best_match = 0;
        for (usize j = 0; j < palette_flag.opts_num; j++) {
            u16 diff_total = (u16)abs(data[i + 0] - palette[j].r) +
                             (u16)abs(data[i + 1] - palette[j].g) +
                             (u16)abs(data[i + 2] - palette[j].b);
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

            i64 target_x = current_x + algorithm->offsets[j].x_offset;
            i64 target_y = current_y + algorithm->offsets[j].y_offset;

            if (target_x < 0 || target_x >= width || target_y < 0 || target_y >= height) continue;

            usize target_i = channels * (target_y * width + target_x);
            i16 new_r = (i16)data[target_i + 0] + (i16)((double)quant_err[0] * algorithm->offsets[j].factor);
            i16 new_g = (i16)data[target_i + 1] + (i16)((double)quant_err[1] * algorithm->offsets[j].factor);
            i16 new_b = (i16)data[target_i + 2] + (i16)((double)quant_err[2] * algorithm->offsets[j].factor);

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
        return 1;
    }

    printf("%s: wrote image of size %dx%d to '%s'\n", ARGS_BINARY_NAME, width, height, output_path);

    stbi_image_free(data);
    return 0;
}

int main(int argc, char **argv) {
    error e = main_wrapper(argc, argv);
    return e;
}
