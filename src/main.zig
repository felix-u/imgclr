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
    // @Fixme { Very bright areas stay bright. (???) }
    if (res.args.invert) {
        var idx: usize = 0;
        while (idx < image.data.len) : (idx += image.num_components) {
            // var row = (idx / 3 / image.width) + 1;
            // var col = (idx / 3 % image.width) + 1;
            // print("{}x{}: {} {} {}\n", .{col, row, image.data[idx + 0], image.data[idx + 1], image.data[idx + 2]});
            var brightness: i16 = (image.data[idx + 0] +| image.data[idx + 1] +| image.data[idx + 2]) / 3;
            var r_relative: i16 = image.data[idx + 0] - brightness;
            var g_relative: i16 = image.data[idx + 1] - brightness;
            var b_relative: i16 = image.data[idx + 2] - brightness;
            var r_new: i16 = 255 - brightness + r_relative;
            var g_new: i16 = 255 - brightness + g_relative;
            var b_new: i16 = 255 - brightness + b_relative;
            if (r_new > 255) {
                r_new = 255;
            }
            else if (r_new < 0) {
                r_new = 0;
            }
            if (g_new > 255) {
                g_new = 255;
            }
            else if (g_new < 0) {
                g_new = 0;
            }
            if (b_new > 255) {
                b_new = 255;
            }
            else if (b_new < 0) {
                b_new = 0;
            }
            image.data[idx + 0] = @intCast(u8, r_new);
            image.data[idx + 1] = @intCast(u8, g_new);
            image.data[idx + 2] = @intCast(u8, b_new);
        }
    }

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
