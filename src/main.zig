const c = @cImport({
    @cDefine("CLR_IMPLEMENATION", "");
    @cInclude("clr.h");
    // Issues with translate-c, but I was going to rewrite this anyway.
    // @cDefine("DITHER_IMPLEMENTATION", "");
    // @cInclude("dither.h");
    @cDefine("STBI_ONLY_JPEG", "");
    @cDefine("STBI_ONLY_PNG", "");
    @cDefine("STBI_ONLY_BMP", "");
    @cDefine("STBI_ONLY_PNM", "");
    @cDefine("STBI_IMAGE_IMPLEMENTATION", "");
    @cDefine("STBI_IMAGE_WRITE_IMPLEMENTATION", "");
    @cDefine("STBI_FAILURE_USERMSG", "");
    @cInclude("stb_image-v2.27/stb_image.h");
    @cInclude("stb_image_write-v1.16/stb_image_write.h");
});
const clap = @import("clap");
const std = @import("std");

const debug = std.debug;


pub fn main() !void {

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
        try clap.usage(std.io.getStdErr().writer(), clap.Help, &params);
        debug.print("\n", .{});
        return clap.help(std.io.getStdErr().writer(), clap.Help, &params, .{});
    }
    if (res.args.version) {
        debug.print("imgclr 0.2-dev\n", .{});
        return;
    }

}

