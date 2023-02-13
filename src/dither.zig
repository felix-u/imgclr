pub const Error = struct {
    x_offset: isize,
    y_offset: isize,
    ratio: f64,
};

pub const Algorithm = struct {
    name: []const u8,
    errors: []const Error,
};


pub const default_algorithms = struct {
    floyd_steinberg: Algorithm = floyd_steinberg,
    atkinson:        Algorithm = atkinson,
    jjn:             Algorithm = jjn,
    burkes:          Algorithm = burkes,
    sierra_lite:     Algorithm = sierra_lite,
} {};


// https://en.wikipedia.org/wiki/Floyd–Steinberg_dithering
//   * 7
// 3 5 1
// multiplier: 1/16
pub const floyd_steinberg: Algorithm = .{
    .name = "floyd-steinberg",
    .errors = .{
        .{ .x_offset =  1, .y_offset = 0, .ratio = 7/16 },
        .{ .x_offset = -1, .y_offset = 1, .ratio = 3/16 },
        .{ .x_offset =  0, .y_offset = 1, .ratio = 5/16 },
        .{ .x_offset =  1, .y_offset = 1, .ratio = 1/16 },
    },
};

// https://gaz.github.io/canvas-atkinson-dither
//   * 1 1
// 1 1 1
//   1
// multiplier: 1/8
pub const atkinson: Algorithm = .{
    .name = "atkinson",
    .errors = .{
        .{ .x_offset =  1, .y_offset = 0, .ratio = 1/8},
        .{ .x_offset =  2, .y_offset = 0, .ratio = 1/8},
        .{ .x_offset = -1, .y_offset = 1, .ratio = 1/8},
        .{ .x_offset =  0, .y_offset = 1, .ratio = 1/8},
        .{ .x_offset =  1, .y_offset = 1, .ratio = 1/8},
        .{ .x_offset =  1, .y_offset = 2, .ratio = 1/8}
    },
};

// https://en.wikipedia.org/wiki/Error_diffusion
//     * 7 5
// 3 5 7 5 3
// 1 3 5 3 1
// multiplier = 1/48
pub const jjn: Algorithm = .{
    .name = "jjn",
    .errors = .{
        .{ .x_offset =  1, .y_offset = 0, .ratio = 7/48},
        .{ .x_offset =  2, .y_offset = 0, .ratio = 5/48},
        .{ .x_offset = -2, .y_offset = 1, .ratio = 3/48},
        .{ .x_offset = -1, .y_offset = 1, .ratio = 5/48},
        .{ .x_offset =  0, .y_offset = 1, .ratio = 7/48},
        .{ .x_offset =  1, .y_offset = 1, .ratio = 5/48},
        .{ .x_offset =  2, .y_offset = 1, .ratio = 3/48},
        .{ .x_offset = -2, .y_offset = 2, .ratio = 1/48},
        .{ .x_offset = -1, .y_offset = 2, .ratio = 3/48},
        .{ .x_offset =  0, .y_offset = 2, .ratio = 5/48},
        .{ .x_offset =  1, .y_offset = 2, .ratio = 3/48},
        .{ .x_offset =  2, .y_offset = 2, .ratio = 1/48}
    },
};

// https://tannerhelland.com/2012/12/28/dithering-eleven-algorithms-source-code.html
//     * 8 4
// 2 4 8 4 2
// multiplier = 1/32
pub const burkes: Algorithm = .{
    .name = "burkes",
    .errors = .{
        .{ .x_offset =  1, .y_offset = 0, .ratio = 8/32},
        .{ .x_offset =  2, .y_offset = 0, .ratio = 4/32},
        .{ .x_offset = -2, .y_offset = 1, .ratio = 2/32},
        .{ .x_offset = -1, .y_offset = 1, .ratio = 4/32},
        .{ .x_offset =  0, .y_offset = 1, .ratio = 8/32},
        .{ .x_offset =  1, .y_offset = 1, .ratio = 4/32},
        .{ .x_offset =  2, .y_offset = 1, .ratio = 2/32}
    },
};

// Very similar to Floyd-Steinberg, and execution speed is *slightly* faster.
pub const sierra_lite: Algorithm = .{
    .name = "sierra-lite",
    .errors = .{
        .{ .x_offset =  1, .y_offset = 0, .ratio = 2/4},
        .{ .x_offset = -1, .y_offset = 1, .ratio = 1/4},
        .{ .x_offset =  0, .y_offset = 1, .ratio = 1/4}
    },
};
