// const c = @cImport({
//     @cDefine ( "STBI_ONLY_JPEG", "" );
//     @cDefine ( "STBI_ONLY_PNG", "" );
//     @cDefine ( "STBI_ONLY_BMP", "" );
//     @cDefine ( "STBI_ONLY_PNM", "" );
//     @cDefine ( "STBI_FAILURE_USERMSG", "" );
//     @cInclude ("stb_image-v2.27/stb_image.h");
//     @cInclude ("stb_image_write-v1.16/stb_image_write.h");
// });

const clap = @import("clap"); // @Enhancement { Replace clap };
const clr = @import("./colour.zig");
const dither = @import("./dither.zig");
const std = @import("std");
const zstbi = @import("zstbi");

const debug = std.debug;
const print = std.debug.print;
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
            if (std.mem.eql(u8, user_str, algorithm.name)) {
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
    var image = zstbi.Image.init(infile, desired_channels) catch |err| {
        print("{s}: error loading image '{s}'\n", .{binary_name, infile});
        print("{}\n", .{err});
        std.os.exit(@enumToInt(errors.unavailable));
    };
    defer image.deinit();
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
        while (idx < image.data.len) : (idx += image.num_components) {
            const r = image.data[idx + 0];
            const g = image.data[idx + 1];
            const b = image.data[idx + 2];
            const brightness: u8 = @intCast(u8, (@as(u16, r) + @as(u16, g) + @as(u16, b)) / 3);
            const r_rel: i16 = @as(i16, r) - brightness;
            const g_rel: i16 = @as(i16, g) - brightness;
            const b_rel: i16 = @as(i16, b) - brightness;
            image.data[idx + 0] = std.math.lossyCast(u8, 255 - brightness +| r_rel);
            image.data[idx + 1] = std.math.lossyCast(u8, 255 - brightness +| g_rel);
            image.data[idx + 2] = std.math.lossyCast(u8, 255 - brightness +| b_rel);
        }
        print("Done!\n", .{});
    }

    // Palette conversion

    print("Matching to palette... ", .{});
    const palette_channels = palette.slice();
    const palette_rs = palette_channels.items(.r);
    const palette_gs = palette_channels.items(.g);
    const palette_bs = palette_channels.items(.b);
    var idx: usize = 0;
    while (idx < image.data.len) : (idx += image.num_components) {
        const image_r: u8 = image.data[idx + 0];
        const image_g: u8 = image.data[idx + 1];
        const image_b: u8 = image.data[idx + 2];

        var min_diff: u16 = std.math.maxInt(u16);
        var best_match: usize = undefined;
        var palette_idx: usize = 0;
        while (palette_idx < palette.len) : (palette_idx += 1) {
            const diff_r: u8 =
                std.math.lossyCast(u8, std.math.absCast(@as(i9, image_r) - @as(i9, palette_rs[palette_idx])));
            const diff_g: u8 =
                std.math.lossyCast(u8, std.math.absCast(@as(i9, image_g) - @as(i9, palette_gs[palette_idx])));
            const diff_b: u8 =
                std.math.lossyCast(u8, std.math.absCast(@as(i9, image_b) - @as(i9, palette_bs[palette_idx])));
            const diff_total: u16 = @as(u16, diff_r) + @as(u16, diff_g) + @as(u16, diff_b);
            if (diff_total < min_diff) {
                min_diff = diff_total;
                best_match = palette_idx;
            }
        }

        image.data[idx + 0] = palette_rs[best_match];
        image.data[idx + 1] = palette_gs[best_match];
        image.data[idx + 2] = palette_bs[best_match];
    }
    print("Done!\n", .{});

    // Write to file
    // @Missing { Handle format - don't just use PNG no matter what }
    zstbi.Image.writeToFile(&image, outfile, .png) catch |err| {
        print("{s}: error writing image to file '{s}'\n", .{binary_name, outfile});
        print("{}\n", .{err});
        std.os.exit(@enumToInt(errors.unavailable));
    };

    print("Wrote {}x{} pixels ({} bytes) to {s}\n", .{image.width, image.height, image.data.len, outfile});
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
