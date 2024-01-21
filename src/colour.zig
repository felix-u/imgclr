const std = @import("std");

pub const Rgb = struct { r: u8, g: u8, b: u8 };

const Error = error{InvalidHexColour};

pub fn rgbFromHexString(
    err_writer: std.fs.File.Writer,
    string: []const u8,
) !Rgb {
    const hex = if (string[0] == '#') string[1..] else string;
    switch (hex.len) {
        3 => {
            const value = std.fmt.parseInt(u12, hex, 16) catch {
                return errInvalid(err_writer, string);
            };
            const r: u8 = @intCast((value & 0xf00) >> 8);
            const g: u8 = @intCast((value & 0x0f0) >> 4);
            const b: u8 = @intCast(value & 0x00f);
            return .{ .r = r * 16 + r, .g = g * 16 + g, .b = b * 16 + b };
        },
        6 => {
            const value = std.fmt.parseInt(u24, hex, 16) catch {
                return errInvalid(err_writer, string);
            };
            return .{
                .r = @intCast((value & 0xff0000) >> 16),
                .g = @intCast((value & 0x00ff00) >> 8),
                .b = @intCast(value & 0x0000ff),
            };
        },
        else => return errInvalid(err_writer, string),
    }
}

fn errInvalid(err_writer: std.fs.File.Writer, string: []const u8) anyerror {
    try err_writer.print("error: invalid hex colour '{s}'\n", .{string});
    return Error.InvalidHexColour;
}
