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

    zstbi.init(allocator);
    defer zstbi.deinit();

    const desired_channels = 3;
    var image = zstbi.Image.init(infile, desired_channels) catch {
        print("{s}: error loading image '{s}'\n", .{binary_name, infile});
        std.os.exit(@enumToInt(errors.unavailable));
    };
    defer image.deinit();

    var palette: []clr.Rgb = try allocator.alloc(clr.Rgb, res.args.palette.len);
    for (res.args.palette) |arg, idx| {
        if (clr.hexToRgb(arg)) |rgb| {
            palette[idx] = rgb;
        }
        else {
            print("{s}: '{s}' is not a valid hex colour\n", .{binary_name, arg});
            std.os.exit(@enumToInt(errors.usage));
        }
    }


    print("{}x{}\n", .{image.width, image.height});

    print("Loaded image {s}\n", .{infile});
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
