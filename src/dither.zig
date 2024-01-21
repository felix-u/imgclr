const colour = @import("./colour.zig");
const std = @import("std");

pub const Algorithm = enum {
    @"floyd-steinberg",
    none,
    atkinson,
    jjn,
    burkes,
    @"sierra-lite",
};

pub fn algorithmFromString(
    err_writer: std.fs.File.Writer,
    string: []const u8,
) !Algorithm {
    inline for (@typeInfo(Algorithm).Enum.fields) |field| {
        if (std.ascii.eqlIgnoreCase(string, field.name)) {
            return @enumFromInt(field.value);
        }
    }
    try err_writer.print(
        "error: dither algorithm '{s}' is invalid\n",
        .{string},
    );
    const Error = error{InvalidAlgorithm};
    return Error.InvalidAlgorithm;
}

const Offset = struct { x: isize, y: isize, factor: f32 };

const @"floyd-steinberg" = [_]Offset{
    .{ .x = 1, .y = 0, .factor = 7.0 / 16.0 },
    .{ .x = -1, .y = 1, .factor = 3.0 / 16.0 },
    .{ .x = 0, .y = 1, .factor = 5.0 / 16.0 },
    .{ .x = 1, .y = 1, .factor = 1.0 / 16.0 },
};

const none = [_]Offset{.{ .x = 0, .y = 0, .factor = 0 }};

const atkinson = [_]Offset{
    .{ .x = 1, .y = 0, .factor = 1.0 / 8.0 },
    .{ .x = 2, .y = 0, .factor = 1.0 / 8.0 },
    .{ .x = -1, .y = 1, .factor = 1.0 / 8.0 },
    .{ .x = 0, .y = 1, .factor = 1.0 / 8.0 },
    .{ .x = 1, .y = 1, .factor = 1.0 / 8.0 },
    .{ .x = 1, .y = 2, .factor = 1.0 / 8.0 },
};

const jjn = [_]Offset{
    .{ .x = 1, .y = 0, .factor = 7.0 / 48.0 },
    .{ .x = 2, .y = 0, .factor = 5.0 / 48.0 },
    .{ .x = -2, .y = 1, .factor = 3.0 / 48.0 },
    .{ .x = -1, .y = 1, .factor = 5.0 / 48.0 },
    .{ .x = 0, .y = 1, .factor = 7.0 / 48.0 },
    .{ .x = 1, .y = 1, .factor = 5.0 / 48.0 },
    .{ .x = 2, .y = 1, .factor = 3.0 / 48.0 },
    .{ .x = -2, .y = 2, .factor = 1.0 / 48.0 },
    .{ .x = -1, .y = 2, .factor = 3.0 / 48.0 },
    .{ .x = 0, .y = 2, .factor = 5.0 / 48.0 },
    .{ .x = 1, .y = 2, .factor = 3.0 / 48.0 },
    .{ .x = 2, .y = 2, .factor = 1.0 / 48.0 },
};

const burkes = [_]Offset{
    .{ .x = 1, .y = 0, .factor = 8.0 / 32.0 },
    .{ .x = 2, .y = 0, .factor = 4.0 / 32.0 },
    .{ .x = -2, .y = 1, .factor = 2.0 / 32.0 },
    .{ .x = -1, .y = 1, .factor = 4.0 / 32.0 },
    .{ .x = 0, .y = 1, .factor = 8.0 / 32.0 },
    .{ .x = 1, .y = 1, .factor = 4.0 / 32.0 },
    .{ .x = 2, .y = 1, .factor = 2.0 / 32.0 },
};

const @"sierra-lite" = [_]Offset{
    .{ .x = 1, .y = 0, .factor = 2.0 / 4.0 },
    .{ .x = -1, .y = 1, .factor = 1.0 / 4.0 },
    .{ .x = 0, .y = 1, .factor = 1.0 / 4.0 },
};

pub fn quantise(
    algorithm: Algorithm,
    data: []u8,
    width: usize,
    height: usize,
    palette: []const colour.Rgb,
) void {
    switch (algorithm) {
        inline else => |a| quantiseAlgorithm(a, data, width, height, palette),
    }
}

fn quantiseAlgorithm(
    comptime algorithm: Algorithm,
    data: []u8,
    width: usize,
    height: usize,
    palette: []const colour.Rgb,
) void {
    var i: usize = 0;
    while (i < data.len) : (i += 3) {
        const pixel: @Vector(3, i16) = data[i .. i + 3][0..3].*;
        var min_diff: u16 = std.math.maxInt(u16);
        var closest_clr = palette[0];
        for (palette) |clr| {
            const diff: u16 = @reduce(.Add, @abs(pixel - clr));
            if (diff >= min_diff) continue;
            min_diff = diff;
            closest_clr = clr;
        }

        data[i .. i + 3][0..3].* = closest_clr;

        switch (algorithm) {
            inline .none => continue,
            inline else => {},
        }

        const quant_error = pixel - closest_clr;

        const channels = 3;
        const this_x: usize = (i / channels) % width;
        const this_y: usize = (i / channels) / width;

        const offsets = @field(@This(), @tagName(algorithm));
        for (offsets) |offset| {
            // NOTE: repeated non-rigorous testing indicates that extracting
            // these bounds checks into separate loops does not speed anything
            // up. In fact, we get a lot slower. I could be doing something
            // wrong, but for now I consider this not worth the effort.
            const x_overflow = @as(isize, @intCast(this_x)) + offset.x;
            const y_overflow = @as(isize, @intCast(this_y)) + offset.y;
            if (x_overflow < 0 or x_overflow >= width or
                y_overflow < 0 or y_overflow >= height)
            {
                continue;
            }

            const target_x: usize = @intCast(x_overflow);
            const target_y: usize = @intCast(y_overflow);
            const target_i = channels * (target_y * width + target_x);

            const err: @Vector(3, f32) = @floatFromInt(quant_error);
            const correction: @Vector(3, i16) = @intFromFloat(
                err * @as(@Vector(3, f32), @splat(offset.factor)),
            );

            var new_pixel: @Vector(3, u8) =
                data[target_i .. target_i + 3][0..3].*;
            inline for (0..3) |chan| new_pixel[chan] =
                std.math.lossyCast(u8, new_pixel[chan] +| correction[chan]);

            data[target_i .. target_i + 3][0..3].* = new_pixel;
        }
    }
}
