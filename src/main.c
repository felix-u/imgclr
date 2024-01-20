#include "base.c"

#define version_lit "0.2-dev"

const Str8 version_text = str8("imgclr version " version_lit "\n");

const Str8 help_text = str8(
"imgclr - image colouriser (version " version_lit ")\n"
"\n"
"Usage: imgclr <input file> <output file> <palette...> [options]\n"
"\n"
"Options:\n"
"      --dither <algorithm>\n"
"        Specify dithering algorithm - one of:\n"
"            'floyd-steinberg' (default), 'none', 'atkinson', 'jjn', \n"
"            'burkes', 'sierra-lite'\n"
"      --invert\n"
"        Invert the image's luminance\n"
"      --palette <hex>...\n"
"        Specify palette - at least two (2) space-separated hex colours\n"
"  -h, --help\n"
"        Print this help and exit\n"
"      --version\n"
"        Print version information and exit\n"
);

#include "args.c"
#include "colour.c"
#include "dither.c"

#ifndef DEBUG
    #include "stbi.c"
#else
    #include "stb_image.h"
    #include "stb_image_write.h"
#endif // DEBUG

typedef enum { FORMAT_JPG, FORMAT_PNG, FORMAT_BMP } Format;

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
    Slice(Rgb) palette;
    uchar *data;
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
    
    if (str8_eql(ext, str8("jpg")) || str8_eql(ext, str8("JPG")) ||
        str8_eql(ext, str8("jpeg")) || str8_eql(ext, str8("JPEG"))
    ) {
        *format = FORMAT_JPG;
    } else if (str8_eql(ext, str8("png")) || str8_eql(ext, str8("PNG"))) {
        *format = FORMAT_PNG;
    } else if (str8_eql(ext, str8("bmp")) || str8_eql(ext, str8("BMP")) ||
        str8_eql(ext, str8("dib")) || str8_eql(ext, str8("DIB"))
    ) {
        *format = FORMAT_BMP;
    } else return errf(
        "extension '%.*s' does not match any supported image format", 
        str8_fmt(ext)
    );
    
    return 0;
}

static error main_wrapper(Context *ctx) {
    try (arena_init(&ctx->arena, 16 * 1024 * 1024));

    Args_Flag invert_flag = { .name = str8("invert") };
    Args_Flag dither_flag = { 
        .name = str8("dither"), 
        .kind = args_kind_single_pos, 
    };
    Args_Flag palette_flag = {
        .name = str8("palette"),
        .kind = args_kind_multi_pos,
    };
    Args_Flag help_flag_short = { .name = str8("h") };
    Args_Flag help_flag_long = { .name = str8("help") };
    Args_Flag version_flag = { .name = str8("version") };
    Args_Flag *flags[] = { 
        &dither_flag,
        &invert_flag, 
        &palette_flag,
        &help_flag_short, &help_flag_long,
        &version_flag,
    };
    Args_Desc args_desc = {
        .exe_kind = args_kind_multi_pos,
        .flags = slice(flags),
    };
    try (args_parse(&ctx->arena, ctx->argc, ctx->argv, &args_desc) != 0);

    if (help_flag_short.is_present || help_flag_long.is_present) {
        printf("%.*s", str8_fmt(help_text));
        return 0;
    }

    if (version_flag.is_present) {
        printf("%.*s", str8_fmt(version_text));
        return 0;
    }

    if (args_desc.multi_pos.len < 2) return err(
        "expected input and output paths as positional arguments"
    );

    if (!palette_flag.is_present || palette_flag.multi_pos.len < 2) {
        return err("expected at least two (2) palette colours");
    }

    ctx->infile_path = args_desc.multi_pos.ptr[0];
    ctx->outfile_path = args_desc.multi_pos.ptr[1];

    try (format_from_str(ctx->outfile_path, &ctx->outfile_format));

    Dither_Algorithm algorithm = floyd_steinberg;
    if (dither_flag.is_present) {
        Str8 s = dither_flag.single_pos;
        if (str8_eql(s, str8("floyd-steinberg"))) {
            algorithm = floyd_steinberg;
        } else if (str8_eql(s, str8("none"))) {
            algorithm = none;
        } else if (str8_eql(s, str8("atkinson"))) {
            algorithm = atkinson;
        } else if (str8_eql(s, str8("jjn"))) {
            algorithm = jjn;
        } else if (str8_eql(s, str8("burkes"))) {
            algorithm = burkes;
        } else if (str8_eql(s, str8("sierra-lite"))) {
            algorithm = sierra_lite;
        } else return errf(
            "invalid algorithm '%.*s'", 
            str8_fmt(dither_flag.single_pos)
        );
    }

    Slice_Str8 colours = palette_flag.multi_pos;
    try (
        arena_alloc(&ctx->arena, colours.len * sizeof(Rgb), &ctx->palette.ptr)
    );
    for (usize i = 0; i < colours.len; i += 1) {
        Rgb rgb; try (rgb_from_hex_str8(colours.ptr[i], &rgb));
        slice_push(ctx->palette, rgb);
    }

    Str8 infile_buf; try (
        read_file(&ctx->arena, ctx->infile_path, "rb", &infile_buf)
    );

    int width = 0, height = 0, channels = 0;
    ctx->data = stbi_load_from_memory(
        infile_buf.ptr, 
        (int)infile_buf.len,
        &width, 
        &height, 
        &channels, 
        3
    );
    if (ctx->data == NULL) return errf(
        "error loading '%.*s':\n%s", 
        str8_fmt(ctx->infile_path), stbi_failure_reason()
    );

    usize data_len = width * height * channels;

    uchar *data = ctx->data;
    if (invert_flag.is_present) for (usize i = 0; i < data_len; i += 3) {
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
        for (usize j = 0; j < algorithm.len; j++) {
            i64 target_x = current_x + algorithm.ptr[j].x_offset;
            i64 target_y = current_y + algorithm.ptr[j].y_offset;
            if (target_x < 0 || target_x >= width || 
                target_y < 0 || target_y >= height
            ) {
                continue;
            }

            usize target_i = channels * (target_y * width + target_x);
            i16 new_r = (i16)data[target_i + 0] + 
                (i16)((double)quant_err[0] * algorithm.ptr[j].factor);
            i16 new_g = (i16)data[target_i + 1] + 
                (i16)((double)quant_err[1] * algorithm.ptr[j].factor);
            i16 new_b = (i16)data[target_i + 2] + 
                (i16)((double)quant_err[2] * algorithm.ptr[j].factor);

            clamp(new_r, 0, 255);
            clamp(new_g, 0, 255);
            clamp(new_b, 0, 255);

            data[target_i + 0] = (u8)new_r;
            data[target_i + 1] = (u8)new_g;
            data[target_i + 2] = (u8)new_b;
        }
    }
    ctx->data = data;

    bool write_ok = false;
    switch (ctx->outfile_format) {
        case FORMAT_JPG: {
            write_ok = stbi_write_jpg(
                (const char *)ctx->outfile_path.ptr, 
                width, 
                height, 
                channels, 
                ctx->data, 
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
                ctx->data, 
                stride_in_bytes
            );
        } break;
        case FORMAT_BMP: {
            write_ok = stbi_write_bmp(
                (const char *)ctx->outfile_path.ptr, 
                width, 
                height, 
                channels, 
                ctx->data
            );
        } break;
    }

    if (!write_ok) return errf(
        "error writing image '%.*s'\n", 
        str8_fmt(ctx->outfile_path)
    );

    printf(
        "wrote image of size %dx%d to '%.*s'\n", 
        width, height, str8_fmt(ctx->outfile_path)
    );
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("%.*s", str8_fmt(help_text));
        return 1;
    }

    Context ctx = { .argc = argc, .argv = argv };
    error e = main_wrapper(&ctx);
    stbi_image_free(ctx.data);
    arena_deinit(&ctx.arena);
    return e;
}
