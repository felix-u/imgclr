#include "base.c"

#define version_lit "0.3"

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
#include "farbfeld.c"

typedef struct {
    Arena arena;
    int argc;
    char **argv;
    Str8 infile;
    Str8 outfile;
    Slice(Rgb) palette;
    Farbfeld_Image image;
} Context;

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
    try (args_parse(ctx->argc, ctx->argv, &args_desc) != 0);

    if (help_flag_short.is_present || help_flag_long.is_present) {
        printf("%.*s", str8_fmt(help_text));
        return 0;
    }

    if (version_flag.is_present) {
        printf("%.*s", str8_fmt(version_text));
        return 0;
    }

    if (args_desc.multi_pos.end_i - args_desc.multi_pos.beg_i < 2) return err(
        "expected input and output paths as positional arguments"
    );

    usize palette_len = 
        palette_flag.multi_pos.end_i - palette_flag.multi_pos.beg_i;
    if (!palette_flag.is_present || palette_len < 2) {
        return err("expected at least two (2) palette colours");
    }

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

    try (
        arena_alloc(&ctx->arena, palette_len * sizeof(Rgb), &ctx->palette.ptr)
    );

    for (
        int i = palette_flag.multi_pos.beg_i; 
        i < palette_flag.multi_pos.end_i; 
        i += 1
    ) {
        Str8 hex_str = str8_from_cstr(ctx->argv[i]);
        Rgb rgb; try (rgb_from_hex_str8(hex_str, &rgb));
        slice_push(ctx->palette, rgb);
    }

    Str8 infile_path = str8_from_cstr(ctx->argv[args_desc.multi_pos.beg_i]);
    Str8 infile_buf; try (
        read_file(&ctx->arena, infile_path, "rb", &infile_buf)
    );

    try (farbfeld_read_from_memory(infile_buf, &ctx->image));
    Farbfeld_Image image = ctx->image;
    u16 *data = image.data;
    usize data_len = image.width * image.height * 8;

    if (invert_flag.is_present) for (usize i = 0; i < data_len; i += 4) {
        i16 brightness = (data[i + 0] + data[i + 1] + data[i + 2]) / 3;
        i16 r_relative = (i16)data[i + 0] - brightness;
        i16 g_relative = (i16)data[i + 1] - brightness;
        i16 b_relative = (i16)data[i + 2] - brightness;

        i16 new_r = (255 - brightness) + r_relative;
        i16 new_g = (255 - brightness) + g_relative;
        i16 new_b = (255 - brightness) + b_relative;

        clamp(new_r, 0, 255); 
        clamp(new_g, 0, 255); 
        clamp(new_b, 0, 255); 

        data[i + 0] = new_r;
        data[i + 1] = new_g;
        data[i + 2] = new_b;
    }

    // NOTE (OUTDATED): Having several loops to avoid bounds checking on the
    // majority of the image is not worth it.

    for (usize i = 0; i < data_len; i += 4) {
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

        usize current_x = (i / 4) % image.width;
        usize current_y = (i / 4) / image.width;
        for (usize j = 0; j < algorithm.len; j++) {
            i64 target_x = current_x + algorithm.ptr[j].x_offset;
            i64 target_y = current_y + algorithm.ptr[j].y_offset;
            if (target_x < 0 || target_x >= image.width || 
                target_y < 0 || target_y >= image.height
            ) {
                continue;
            }

            usize target_i = 4 * (target_y * image.width + target_x);
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
    ctx->image.data = data;

    return err("writing not implemented");
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("%.*s", str8_fmt(help_text));
        return 1;
    }

    Context ctx = { .argc = argc, .argv = argv };
    error e = main_wrapper(&ctx);
    arena_deinit(&ctx.arena);
    return e;
}
