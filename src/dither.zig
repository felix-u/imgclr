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

const Offset = struct { x: isize, y: isize, factor: f64 };

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
        const r = @as(i16, data[i]);
        const g = @as(i16, data[i + 1]);
        const b = @as(i16, data[i + 2]);

        var min_diff: usize = std.math.maxInt(usize);
        var closest_clr_i: usize = 0;
        for (palette, 0..palette.len) |clr, j| {
            const diff_r = @abs(r - clr.r);
            const diff_g = @abs(g - clr.g);
            const diff_b = @abs(b - clr.b);
            const diff: usize = diff_r + diff_g + diff_b;
            if (diff >= min_diff) continue;
            min_diff = diff;
            closest_clr_i = j;
        }

        const closest_clr = palette[closest_clr_i];
        data[i] = closest_clr.r;
        data[i + 1] = closest_clr.g;
        data[i + 2] = closest_clr.b;

        switch (algorithm) {
            inline .none => continue,
            inline else => {},
        }

        const QuantError = struct { r: i16, g: i16, b: i16 };
        const quant_error = QuantError{
            .r = r - closest_clr.r,
            .g = g - closest_clr.g,
            .b = b - closest_clr.b,
        };

        const channels = 3;
        const this_x = (i / channels) % width;
        const this_y = (i / channels) / width;

        const offsets = @field(@This(), @tagName(algorithm));
        for (offsets) |offset| {
            const target_x_overflow = @as(isize, @intCast(this_x)) + offset.x;
            const target_y_overflow = @as(isize, @intCast(this_y)) + offset.y;
            if (target_x_overflow < 0 or target_x_overflow >= width or
                target_y_overflow < 0 or target_y_overflow >= height)
            {
                continue;
            }

            const target_x: usize = @intCast(target_x_overflow);
            const target_y: usize = @intCast(target_y_overflow);
            const target_i = channels * (target_y * width + target_x);

            const r_err: f64 = @floatFromInt(quant_error.r);
            const g_err: f64 = @floatFromInt(quant_error.g);
            const b_err: f64 = @floatFromInt(quant_error.b);

            const r_correction: i16 = @intFromFloat(offset.factor * r_err);
            const g_correction: i16 = @intFromFloat(offset.factor * g_err);
            const b_correction: i16 = @intFromFloat(offset.factor * b_err);

            const r_new =
                std.math.lossyCast(u8, data[target_i] +| r_correction);
            const g_new =
                std.math.lossyCast(u8, data[target_i + 1] +| g_correction);
            const b_new =
                std.math.lossyCast(u8, data[target_i + 2] +| b_correction);

            data[target_i] = r_new;
            data[target_i + 1] = g_new;
            data[target_i + 2] = b_new;
        }
    }
}
