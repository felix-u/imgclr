const args = @import("./args.zig");
const colour = @import("./colour.zig");
const dither = @import("./dither.zig");
const std = @import("std");

const c = @cImport({
    @cInclude("stb_image.h");
    @cInclude("stb_image_write.h");
});

const Error = error{ InvalidUsage, WriteError };

pub fn main() !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    const stdout = std.io.getStdOut();
    const stdout_writer = stdout.writer();
    const stderr = std.io.getStdErr();
    const stderr_writer = stderr.writer();

    const argv = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, argv);

    const args_parsed =
        try args.parseAlloc(allocator, stdout_writer, stderr_writer, argv, .{
        .desc = "image quantiser",
        .ver = "0.3-dev",
        .usage = "<input> <output> <palette>... [options]",
        .cmds = &.{args.Cmd{
            .name = "imgclr",
            .kind = .multi_pos,
            .flags = &.{
                .{
                    .short = 'd',
                    .long = "dither",
                    .desc = "Specify dithering algorithm",
                    .usage = "<algorithm>",
                    .kind = .multi_pos,
                },
                .{
                    .short = 'i',
                    .long = "invert",
                    .desc = "Invert image luminance",
                },
                .{
                    .short = 'p',
                    .long = "palette",
                    .desc = "Specify palette - at least two (2) hex colours",
                    .usage = "<hex>...",
                    .kind = .multi_pos,
                    .required = true,
                },
            },
        }},
    }) orelse return;
    const opts = args_parsed.imgclr;

    if (opts.pos.items.len != 2) {
        try stderr_writer.print("error: expected input and output files as " ++
            "2 positional arguments, but got {d}\n", .{opts.pos.items.len});
        return Error.InvalidUsage;
    }

    if (opts.palette.items.len < 2) {
        try stderr_writer.print(
            "error: expected at least 2 palette colours, but got {d}\n",
            .{opts.palette.items.len},
        );
        return Error.InvalidUsage;
    }

    if (opts.dither.items.len > 1) {
        try stderr_writer.print(
            "error: unexpected positional argument '{s}'\n",
            .{opts.dither.items[1]},
        );
        return Error.InvalidUsage;
    }

    const dither_algorithm = if (opts.dither.items.len == 0)
        dither.Algorithm.@"floyd-steinberg"
    else
        try dither.algorithmFromString(stderr_writer, opts.dither.items[0]);

    var palette = try std.ArrayList(colour.Rgb).initCapacity(
        allocator,
        opts.palette.items.len,
    );
    defer palette.deinit();
    for (opts.palette.items) |hex_string| {
        const rgb = try colour.rgbFromHexString(stderr_writer, hex_string);
        palette.appendAssumeCapacity(rgb);
    }

    const infile_path = opts.pos.items[0];
    const infile = try readFileAlloc(allocator, infile_path);

    const outfile_path = opts.pos.items[1];
    const outfile_format =
        try imageFormatFromFilename(stderr_writer, outfile_path);

    var width: c_int = 0;
    var height: c_int = 0;
    var channels: c_int = 0;
    const data_ptr = c.stbi_load_from_memory(
        @ptrCast(infile),
        @intCast(infile.len),
        &width,
        &height,
        &channels,
        3,
    );
    defer c.stbi_image_free(data_ptr);

    const data_len = width * height * channels;
    const data = data_ptr[0..@intCast(data_len)];
    dither.quantise(
        dither_algorithm,
        data,
        @intCast(width),
        @intCast(height),
        palette.items,
    );

    const write_ok = switch (outfile_format) {
        .jpg => c.stbi_write_jpg(
            @ptrCast(outfile_path),
            width,
            height,
            channels,
            data_ptr,
            100,
        ),
        .png => c.stbi_write_png(
            @ptrCast(outfile_path),
            width,
            height,
            channels,
            data_ptr,
            width * channels,
        ),
        .bmp => c.stbi_write_bmp(
            @ptrCast(outfile_path),
            width,
            height,
            channels,
            data_ptr,
        ),
    };
    if (write_ok == 0) {
        try stderr_writer.print(
            "error: failure writing image to location '{s}'\n",
            .{outfile_path},
        );
        return Error.WriteError;
    }

    try stdout_writer.print(
        "wrote image of size {d}x{d} to '{s}'\n",
        .{ width, height, outfile_path },
    );
}

fn readFileAlloc(
    allocator: std.mem.Allocator,
    filepath: []const u8,
) ![]const u8 {
    const cwd = std.fs.cwd();
    const infile = try cwd.openFile(filepath, .{ .mode = .read_only });
    defer infile.close();
    return try infile.readToEndAlloc(allocator, std.math.maxInt(u32));
}

const ImageFormat = enum { jpg, png, bmp };
const ImageFormatParsingError = error{UnknownExtension};
fn imageFormatFromFilename(
    err_writer: std.fs.File.Writer,
    filename: []const u8,
) !ImageFormat {
    const extension_i = std.mem.indexOfScalar(u8, filename, '.') orelse {
        try err_writer.print("error: unable to infer image format " ++
            "from file name '{s}' with no extension\n", .{filename});
        return ImageFormatParsingError.UnknownExtension;
    };
    const extension = filename[extension_i..];

    if (std.ascii.eqlIgnoreCase(extension, ".jpg") or
        std.ascii.eqlIgnoreCase(extension, ".jpeg"))
    {
        return .jpg;
    } else if (std.ascii.eqlIgnoreCase(extension, ".png")) {
        return .png;
    } else if (std.ascii.eqlIgnoreCase(extension, ".bmp") or
        std.ascii.eqlIgnoreCase(extension, ".dib"))
    {
        return .bmp;
    } else {
        try err_writer.print(
            "error: unable to infer image format " ++
                "from file name '{s}' with extension '{s}'",
            .{ filename, extension },
        );
        return ImageFormatParsingError.UnknownExtension;
    }
}
