#include "base.c"

#include "colour.c"
#include "dither.c"

#ifndef DEBUG
    #include "stbi.c"
#else
    #include "stb_image.h"
    #include "stb_image_write.h"
#endif // DEBUG

typedef enum {
    FORMAT_JPG,
    FORMAT_PNG,
    FORMAT_BMP,
} Format;

typedef struct {
    Arena arena;
    int argc;
    char **argv;
    Str8 infile_path;
    Str8 infile;
    Format infile_format;
    Str8 outfile_path;
    Str8 outfile;
    Format outfile_format;
    Str8 dither_arg;
    i8 swap_arg;
    Slice(Rgb) palette;
} Context;

static error format_from_str(Str8 str, Format *format) {
    usize extension_pos = str.len;
    for (usize i = str.len; i >= 0; i--) {
        if (str.ptr[i] != '.') continue;
        extension_pos = i + 1;
        break;
    }
    
    if (extension_pos + 1 >= str.len) return errf(
        "unable to infer image format from filename '%.*s'", 
        str8_fmt(str)
    );

    Str8 ext = str8_range(str, extension_pos, str.len);
    
    if (str8_eql(ext, str8("jpg")) || str8_eql(ext, str8("jpeg"))) {
        *format = FORMAT_JPG;
    } else if (str8_eql(ext, str8("png"))) {
        *format = FORMAT_PNG;
    } else if (str8_eql(ext, str8("bmp")) || str8_eql(ext, str8("dib"))) {
        *format = FORMAT_BMP;
    } else return errf(
        "extension '%.*s' does not match any supported image format", 
        str8_fmt(ext)
    );
    
    return 0;
}

static error main_wrapper(Context *ctx) {
    try (arena_init(&ctx->arena, 16 * 1024 * 1024));
    try (format_from_str(ctx->outfile_path, &ctx->outfile_format));

    const Dither_Algorithm *algorithm = &floyd_steinberg;
    bool ok = false;
    for (usize i = 0; i < DITHER_ALGORITHM_NUM; i++) {
        if (!str8_eql(ctx->dither_arg, DITHER_ALGORITHMS[i]->name)) continue;
        ok = true;
        algorithm = DITHER_ALGORITHMS[i];
        break;
    }
    if (!ok) return errf("no algorithm '%.*s'", str8_fmt(ctx->dither_arg));

    int palette_arg_i = 5;
    int palette_num = ctx->argc - palette_arg_i;
    try (
        arena_alloc(&ctx->arena, palette_num * sizeof(Rgb), &ctx->palette.ptr)
    );
    for (int i = palette_arg_i; i < ctx->argc; i += 1) {
        Rgb rgb; try (hex_to_rgb(ctx->argv[i], &rgb));
        slice_push(ctx->palette, rgb);
    }

    int width = 0, height = 0, channels = 0;
    uchar *data = stbi_load(
        (const char *)ctx->infile_path.ptr, 
        &width, 
        &height, 
        &channels, 
        3
    );
    if (data == NULL) return errf(
        "error loading '%.*s':\n%s", 
        str8_fmt(ctx->infile_path), stbi_failure_reason()
    );

    usize data_len = width * height * channels;

    if (ctx->swap_arg) for (usize i = 0; i < data_len; i += 3) {
        i16 brightness = (data[i + 0] + data[i + 1] + data[i + 2]) / 3;
        i16 r_relative = data[i + 0] - brightness;
        i16 g_relative = data[i + 1] - brightness;
        i16 b_relative = data[i + 2] - brightness;

        i16 new_r = (255 - brightness) + r_relative;
        i16 new_g = (255 - brightness) + g_relative;
        i16 new_b = (255 - brightness) + b_relative;

        clamp(new_r, 0, 255); 
        clamp(new_g, 0, 255); 
        clamp(new_b, 0, 255); 

        data[i + 0] = (u8)new_r;
        data[i + 1] = (u8)new_g;
        data[i + 2] = (u8)new_b;
    }

    // NOTE (OUTDATED): Having several loops to avoid bounds checking on the
    // majority of the image is not worth it.

    for (usize i = 0; i < data_len; i += channels) {
        u16 min_diff = 999;
        usize best_match = 0;
        for (usize j = 0; j < ctx->palette.len; j += 1) {
            u16 diff_total = (u16)abs(data[i + 0] - ctx->palette.ptr[j].r) +
                             (u16)abs(data[i + 1] - ctx->palette.ptr[j].g) +
                             (u16)abs(data[i + 2] - ctx->palette.ptr[j].b);
            if (diff_total < min_diff) {
                min_diff = diff_total;
                best_match = j;
            }
        }

        i16 quant_err[3] = {
            (i16)data[i + 0] - ctx->palette.ptr[best_match].r,
            (i16)data[i + 1] - ctx->palette.ptr[best_match].g,
            (i16)data[i + 2] - ctx->palette.ptr[best_match].b
        };

        data[i + 0] = ctx->palette.ptr[best_match].r;
        data[i + 1] = ctx->palette.ptr[best_match].g;
        data[i + 2] = ctx->palette.ptr[best_match].b;

        usize current_x = (i / channels) % width;
        usize current_y = (i / channels) / width;
        for (usize j = 0; j < algorithm->offset_num; j++) {
            i64 target_x = current_x + algorithm->offsets[j].x_offset;
            i64 target_y = current_y + algorithm->offsets[j].y_offset;
            if (target_x < 0 || target_x >= width || 
                target_y < 0 || target_y >= height
            ) {
                continue;
            }

            usize target_i = channels * (target_y * width + target_x);
            i16 new_r = (i16)data[target_i + 0] + 
                (i16)((double)quant_err[0] * algorithm->offsets[j].factor);
            i16 new_g = (i16)data[target_i + 1] + 
                (i16)((double)quant_err[1] * algorithm->offsets[j].factor);
            i16 new_b = (i16)data[target_i + 2] + 
                (i16)((double)quant_err[2] * algorithm->offsets[j].factor);

            clamp(new_r, 0, 255);
            clamp(new_g, 0, 255);
            clamp(new_b, 0, 255);

            data[target_i + 0] = (u8)new_r;
            data[target_i + 1] = (u8)new_g;
            data[target_i + 2] = (u8)new_b;
        }
    }

    bool write_ok = false;
    switch (ctx->outfile_format) {
        case FORMAT_JPG: {
            write_ok = stbi_write_jpg(
                (const char *)ctx->outfile_path.ptr, 
                width, 
                height, 
                channels, 
                data, 
                100
            );
        } break;
        case FORMAT_PNG: {
            int stride_in_bytes = width * channels;
            write_ok = stbi_write_png(
                (const char *)ctx->outfile_path.ptr, 
                width, 
                height, 
                channels, 
                data, 
                stride_in_bytes
            );
        } break;
        case FORMAT_BMP: {
            write_ok = stbi_write_bmp(
                (const char *)ctx->outfile_path.ptr, 
                width, 
                height, 
                channels, 
                data
            );
        } break;
    }

    if (!write_ok) {
        stbi_image_free(data);
        return 
            errf("error writing image '%.*s'\n", str8_fmt(ctx->outfile_path));
    }

    printf(
        "wrote image of size %dx%d to '%.*s'\n", 
        width, height, str8_fmt(ctx->outfile_path)
    );
    stbi_image_free(data);
    return 0;
}

int main(int argc, char **argv) {
    // HACK while I work on a new args.c
    // $ exe infile outfile dither swap palette+
    if (argc < 7) return err(
        "[HACK] expected at least 6 arguments, in this order:\n"
        "infile outfile dither swap palette+"
    );

    Context ctx = {
        .argc = argc,
        .argv = argv,
        .infile_path = str8_from_cstr(argv[1]),
        .outfile_path = str8_from_cstr(argv[2]),
        .dither_arg = str8_from_cstr(argv[3]),
        .swap_arg = argv[4][0] - '0',
    };

    error e = main_wrapper(&ctx);
    arena_deinit(&ctx.arena);
    return e;
}
