pub const Error = struct {
    x_offset: isize,
    y_offset: isize,
    ratio: f64,
};

pub const Algorithm = struct {
    name: []const u8,
    errors: []const Error,
};


pub const default_algorithms: []const Algorithm = &.{
    none,
    floyd_steinberg,
    atkinson,
    jjn,
    burkes,
    sierra_lite,
};


pub const none = Algorithm {
    .name = "none",
    .errors = &.{},
};

// https://en.wikipedia.org/wiki/Floyd–Steinberg_dithering
//   * 7
// 3 5 1
// multiplier: 1/16
pub const floyd_steinberg = Algorithm {
    .name = "floyd-steinberg",
    .errors = &.{
        .{ .x_offset =  1, .y_offset = 0, .ratio = 7.0/16.0 },
        .{ .x_offset = -1, .y_offset = 1, .ratio = 3.0/16.0 },
        .{ .x_offset =  0, .y_offset = 1, .ratio = 5.0/16.0 },
        .{ .x_offset =  1, .y_offset = 1, .ratio = 1.0/16.0 },
    },
};

// https://gaz.github.io/canvas-atkinson-dither
//   * 1 1
// 1 1 1
//   1
// multiplier: 1/8
pub const atkinson = Algorithm {
    .name = "atkinson",
    .errors = &.{
        .{ .x_offset =  1, .y_offset = 0, .ratio = 1.0/8.0 },
        .{ .x_offset =  2, .y_offset = 0, .ratio = 1.0/8.0 },
        .{ .x_offset = -1, .y_offset = 1, .ratio = 1.0/8.0 },
        .{ .x_offset =  0, .y_offset = 1, .ratio = 1.0/8.0 },
        .{ .x_offset =  1, .y_offset = 1, .ratio = 1.0/8.0 },
        .{ .x_offset =  1, .y_offset = 2, .ratio = 1.0/8.0 },
    },
};

// https://en.wikipedia.org/wiki/Error_diffusion
//     * 7 5
// 3 5 7 5 3
// 1 3 5 3 1
// multiplier = 1/48
pub const jjn = Algorithm {
    .name = "jjn",
    .errors = &.{
        .{ .x_offset =  1, .y_offset = 0, .ratio = 7.0/48.0 },
        .{ .x_offset =  2, .y_offset = 0, .ratio = 5.0/48.0 },
        .{ .x_offset = -2, .y_offset = 1, .ratio = 3.0/48.0 },
        .{ .x_offset = -1, .y_offset = 1, .ratio = 5.0/48.0 },
        .{ .x_offset =  0, .y_offset = 1, .ratio = 7.0/48.0 },
        .{ .x_offset =  1, .y_offset = 1, .ratio = 5.0/48.0 },
        .{ .x_offset =  2, .y_offset = 1, .ratio = 3.0/48.0 },
        .{ .x_offset = -2, .y_offset = 2, .ratio = 1.0/48.0 },
        .{ .x_offset = -1, .y_offset = 2, .ratio = 3.0/48.0 },
        .{ .x_offset =  0, .y_offset = 2, .ratio = 5.0/48.0 },
        .{ .x_offset =  1, .y_offset = 2, .ratio = 3.0/48.0 },
        .{ .x_offset =  2, .y_offset = 2, .ratio = 1.0/48.0 },
    },
};

// https://tannerhelland.com/2012/12/28/dithering-eleven-algorithms-source-code.html
//     * 8 4
// 2 4 8 4 2
// multiplier = 1/32
pub const burkes = Algorithm {
    .name = "burkes",
    .errors = &.{
        .{ .x_offset =  1, .y_offset = 0, .ratio = 8.0/32.0 },
        .{ .x_offset =  2, .y_offset = 0, .ratio = 4.0/32.0 },
        .{ .x_offset = -2, .y_offset = 1, .ratio = 2.0/32.0 },
        .{ .x_offset = -1, .y_offset = 1, .ratio = 4.0/32.0 },
        .{ .x_offset =  0, .y_offset = 1, .ratio = 8.0/32.0 },
        .{ .x_offset =  1, .y_offset = 1, .ratio = 4.0/32.0 },
        .{ .x_offset =  2, .y_offset = 1, .ratio = 2.0/32.0 },
    },
};

// Very similar to Floyd-Steinberg, and execution speed is *slightly* faster.
pub const sierra_lite = Algorithm {
    .name = "sierra-lite",
    .errors = &.{
        .{ .x_offset =  1, .y_offset = 0, .ratio = 2.0/4.0 },
        .{ .x_offset = -1, .y_offset = 1, .ratio = 1.0/4.0 },
        .{ .x_offset =  0, .y_offset = 1, .ratio = 1.0/4.0 },
    },
};

