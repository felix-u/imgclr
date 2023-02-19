// const c = @cImport({
//     @cDefine ( "STBI_ONLY_JPEG", "" );
//     @cDefine ( "STBI_ONLY_PNG", "" );
//     @cDefine ( "STBI_ONLY_BMP", "" );
//     @cDefine ( "STBI_ONLY_PNM", "" );
//     @cDefine ( "STBI_FAILURE_USERMSG", "" );
//     @cInclude ("stb_image-v2.27/stb_image.h");
//     @cInclude ("stb_image_write-v1.16/stb_image_write.h");
// });

const args = @import("./args.zig");
const clap = @import("clap"); // @Enhancement { Replace clap };
const clr = @import("./colour.zig");
const dither = @import("./dither.zig");
const std = @import("std");
const zstbi = @import("zstbi");

const ascii = std.ascii;
const debug = std.debug;
const maths = std.math;
const print = std.debug.print;
const process = std.process;
const Soa = std.MultiArrayList;

const binary_name = "imgclr";
const binary_vers = "0.2-dev";

const errors = enum(u8) {
    usage = 64,
    noinput = 66,
    unavailable = 69,
};


pub fn main() !void {

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    // WIP and placeholder
    const argv: []const [:0]u8 = try process.argsAlloc(allocator);
    defer process.argsFree(allocator, argv);

    var dither_flag = args.Flag {
        .name_short   = 'd',
        .name_long    = "dither",
        .help_text    =
            \\specify dithering algorithm, or 'none' to disable. Default is 'floyd-steinberg'.
            \\Other options are: 'atkinson', 'jjn', 'burkes', and 'sierra-lite'
        ,
        .expects_args = .single_arg,
        .args_type    = .string,
    };
    var invert_flag = args.Flag {
        .name_short  = 'i',
        .name_long   = "invert",
        .help_text   = "invert image brightness, preserving hue and saturation",
        .expects_args = .boolean,
    };
    var palette_flag = args.Flag {
        .name_short   = 'p',
        .name_long    = "palette",
        .help_text    = "supply palette in hex form",
        .expects_args = .multiple_args,
        .args_type    = .string,
    };

    args.proc(argv, std.io.getStdErr().writer(), allocator, .{
        .binary_name = "imgclr",
        .binary_ver  = "0.2-dev",
        .usage_desc  = "image colouriser",
        .flags       = &.{
            &dither_flag,
            &invert_flag,
            &palette_flag,
        },
        .expects_pos = .multiple_args,
        .pos_type    = .path,
    }) catch unreachable;

    const params = comptime clap.parseParamsComptime(
        \\-d, --dither <str>        specify dithering algorithm
        \\-i, --invert              invert image brightness, preserving hue and saturation
        \\-p, --palette <str>...    supply palette in hex form
        \\-h, --help                display this help and exit
        \\    --version             display version information and exit
        \\<str>...
        \\
    );

    var diag = clap.Diagnostic{};
    var res = clap.parse(clap.Help, &params, clap.parsers.default, .{
        .diagnostic = &diag,
    }) catch |err| {
        diag.report(std.io.getStdErr().writer(), err) catch {};
        return err;
    };
    defer res.deinit();

    if (res.args.help) {
        try printHelp(&params);
        std.os.exit(0);
    }
    if (res.args.version) {
        print("{s} (version {s})\n", .{binary_name, binary_vers});
        std.os.exit(0);
    }
    if (res.positionals.len < 2) {
        print("{s}: expected filename and destination\n", .{binary_name});
        printHelpHint();
        std.os.exit(@enumToInt(errors.noinput));
    }
    if (res.args.palette.len < 2) {
        print("{s}: expected at least two (2) palette colours\n", .{binary_name});
        printHelpHint();
        std.os.exit(@enumToInt(errors.noinput));
    }

    var dither_algorithm = dither.floyd_steinberg;
    if (res.args.dither) |user_str| {
        var found_match = false;
        for (dither.default_algorithms) |algorithm| {
            if (ascii.eqlIgnoreCase(user_str, algorithm.name)) {
                dither_algorithm = algorithm;
                found_match = true;
                break;
            }
        }
        if (!found_match) {
            print("{s}: '{s}' isn't a recognised dithering algorithm\n", .{binary_name, user_str});
            printHelpHint();
            std.os.exit(@enumToInt(errors.usage));
        }
    }

    const infile = @ptrCast([:0]const u8, res.positionals[0]);
    const outfile = @ptrCast([:0]const u8, res.positionals[1]);

    zstbi.init(allocator);
    defer zstbi.deinit();

    const desired_channels = 3;
    var img = zstbi.Image.init(infile, desired_channels) catch |err| {
        print("{s}: error loading image '{s}'\n", .{binary_name, infile});
        print("{}\n", .{err});
        std.os.exit(@enumToInt(errors.unavailable));
    };
    defer img.deinit();
    print("Loaded image '{s}'\n", .{infile});

    var palette = Soa(clr.Rgb) {};
    try palette.ensureTotalCapacity(allocator, res.args.palette.len);
    for (res.args.palette) |arg| {
        if (clr.hexToRgb(arg)) |rgb| {
            palette.appendAssumeCapacity(rgb);
        }
        else {
            print("{s}: '{s}' is not a valid hex colour\n", .{binary_name, arg});
            std.os.exit(@enumToInt(errors.usage));
        }
    }

    // Invert brightness, if applicable

    if (res.args.invert) {
        print("Inverting brightness... ", .{});
        var idx: usize = 0;
        while (idx < img.data.len) : (idx += img.num_components) {
            const r = img.data[idx + 0];
            const g = img.data[idx + 1];
            const b = img.data[idx + 2];
            const brightness: u8 = @intCast(u8, (@as(u16, r) + @as(u16, g) + @as(u16, b)) / 3);
            const r_rel: i16 = @as(i16, r) - brightness;
            const g_rel: i16 = @as(i16, g) - brightness;
            const b_rel: i16 = @as(i16, b) - brightness;
            img.data[idx + 0] = maths.lossyCast(u8, 255 - brightness +| r_rel);
            img.data[idx + 1] = maths.lossyCast(u8, 255 - brightness +| g_rel);
            img.data[idx + 2] = maths.lossyCast(u8, 255 - brightness +| b_rel);
        }
        print("Done!\n", .{});
    }

    // Palette conversion

    print("Matching to palette... ", .{});
    const palette_channels = palette.slice();
    const palette_rs = palette_channels.items(.r);
    const palette_gs = palette_channels.items(.g);
    const palette_bs = palette_channels.items(.b);
    var i: usize = 0;
    while (i < img.data.len) : (i += img.num_components) {
        const img_r: u8 = img.data[i + 0];
        const img_g: u8 = img.data[i + 1];
        const img_b: u8 = img.data[i + 2];

        var min_diff: u16 = maths.maxInt(u16);
        var best_match: usize = undefined;
        var palette_i: usize = 0;
        while (palette_i < palette.len) : (palette_i += 1) {
            const diff_r: u8 = maths.lossyCast(u8, maths.absCast(@as(i16, img_r) - @as(i16, palette_rs[palette_i])));
            const diff_g: u8 = maths.lossyCast(u8, maths.absCast(@as(i16, img_g) - @as(i16, palette_gs[palette_i])));
            const diff_b: u8 = maths.lossyCast(u8, maths.absCast(@as(i16, img_b) - @as(i16, palette_bs[palette_i])));
            const diff_total: u16 = @as(u16, diff_r) + @as(u16, diff_g) + @as(u16, diff_b);
            if (diff_total < min_diff) {
                min_diff = diff_total;
                best_match = palette_i;
            }
        }

        const quant_err: [3]i16 = .{
            @as(i16, img_r) - palette_rs[best_match],
            @as(i16, img_g) - palette_gs[best_match],
            @as(i16, img_b) - palette_bs[best_match],
        };
        const current_x: isize = @intCast(isize, (i / img.num_components) % img.width);
        const current_y: isize = @intCast(isize, (i / img.num_components) / img.width);
        for (dither_algorithm.errors) |d_error| {
            const target_x: isize = current_x + d_error.x_offset;
            const target_y: isize = current_y + d_error.y_offset;

            if (target_x < 0 or target_x >= img.width or target_y < 0 or target_y >= img.height) continue;

            const target_i: usize =
                img.num_components * (@intCast(usize, target_y) * img.width + @intCast(usize, target_x));
            @setFloatMode(.Optimized);
            var new_r: i16 =
                @as(i16, img.data[target_i + 0]) + @floatToInt(i16, @intToFloat(f64, quant_err[0]) * d_error.ratio);
            var new_g: i16 =
                @as(i16, img.data[target_i + 1]) + @floatToInt(i16, @intToFloat(f64, quant_err[1]) * d_error.ratio);
            var new_b: i16 =
                @as(i16, img.data[target_i + 2]) + @floatToInt(i16, @intToFloat(f64, quant_err[2]) * d_error.ratio);
            img.data[target_i + 0] = maths.lossyCast(u8, new_r);
            img.data[target_i + 1] = maths.lossyCast(u8, new_g);
            img.data[target_i + 2] = maths.lossyCast(u8, new_b);
        }

        img.data[i + 0] = palette_rs[best_match];
        img.data[i + 1] = palette_gs[best_match];
        img.data[i + 2] = palette_bs[best_match];
    }
    print("Done!\n", .{});

    // Write to file
    // @Missing { Handle format - don't just use PNG no matter what }
    print("Writing {}x{} px image to {s}... ", .{img.width, img.height, outfile});
    zstbi.Image.writeToFile(&img, outfile, .png) catch |err| {
        print("{s}: error writing image to file '{s}'\n", .{binary_name, outfile});
        print("{}\n", .{err});
        std.os.exit(@enumToInt(errors.unavailable));
    };
    print("Done!\n", .{});
}


fn printHelpHint() void {
    print("Try {s} --help for more information.\n", .{binary_name});
}


fn printHelp(params: []const clap.Param(clap.Help)) !void {
    print("{s} (version {s})\n", .{binary_name, binary_vers});
    try clap.usage(std.io.getStdErr().writer(), clap.Help, params);
    print("\n\nUSAGE\n", .{});
    return clap.help(std.io.getStdErr().writer(), clap.Help, params, .{});
}

