const std = @import("std");

const Offset = struct { x: isize, y: isize, factor: f64 };

const Algorithm = []const Offset;

pub const algorithm_map = std.ComptimeStringMap(Algorithm, .{
    .{ "floyd-steinberg", &floyd_steinberg },
    .{ "none", &blank },
    .{ "atkinson", &atkinson },
    .{ "jjn", &jjn },
});

pub const floyd_steinberg = [_]Offset{
    .{ .x = 1, .y = 0, .factor = 7 / 16 },
    .{ .x = -1, .y = 1, .factor = 3 / 16 },
    .{ .x = 0, .y = 1, .factor = 5 / 16 },
    .{ .x = 1, .y = 1, .factor = 1 / 16 },
};

pub const blank = [_]Offset{.{ .x = 0, .y = 0, .factor = 0 }};

pub const atkinson = [_]Offset{
    .{ .x = 1, .y = 0, .factor = 1 / 8 },
    .{ .x = 2, .y = 0, .factor = 1 / 8 },
    .{ .x = -1, .y = 1, .factor = 1 / 8 },
    .{ .x = 0, .y = 1, .factor = 1 / 8 },
    .{ .x = 1, .y = 1, .factor = 1 / 8 },
    .{ .x = 1, .y = 2, .factor = 1 / 8 },
};

pub const jjn = [_]Offset{
    .{ .x = 1, .y = 0, .factor = 7 / 48 },
    .{ .x = 2, .y = 0, .factor = 5 / 48 },
    .{ .x = -2, .y = 1, .factor = 3 / 48 },
    .{ .x = -1, .y = 1, .factor = 5 / 48 },
    .{ .x = 0, .y = 1, .factor = 7 / 48 },
    .{ .x = 1, .y = 1, .factor = 5 / 48 },
    .{ .x = 2, .y = 1, .factor = 3 / 48 },
    .{ .x = -2, .y = 2, .factor = 1 / 48 },
    .{ .x = -1, .y = 2, .factor = 3 / 48 },
    .{ .x = 0, .y = 2, .factor = 5 / 48 },
    .{ .x = 1, .y = 2, .factor = 3 / 48 },
    .{ .x = 2, .y = 2, .factor = 1 / 48 },
};
