const std = @import("std");

const ascii = std.ascii;
const fmt = std.fmt;


pub const Rgb = struct {
    r: u8 = 0,
    g: u8 = 0,
    b: u8 = 0,
};


pub fn strIsHex(str: []const u8) bool {
    for (str) |c| {
        if (!ascii.isHex(c)) return false;
    }
    return true;
}


pub fn hexToRgb(str: []const u8) ?Rgb {

    var start_idx: usize = 0;
    var hex_len: usize = 0;

    for (str) |c, idx| {
        if (!ascii.isHex(c)) continue;
        start_idx = idx;
        hex_len = str.len - start_idx;
        // Hex colour must be in either three-digit format - "fff" - or six-digit format - "f0f0f0".
        if (hex_len != 3 and hex_len != 6) return null;
        if (!strIsHex(str[start_idx..])) return null;
        break;
    }

    // Now we've got a valid hex string to parse.

    var byte_buf: [3]u8 = undefined;

    if (hex_len == 3) {
        _ = fmt.hexToBytes(&byte_buf, &[_]u8{
            str[start_idx + 0],
            str[start_idx + 0],
            str[start_idx + 1],
            str[start_idx + 1],
            str[start_idx + 2],
            str[start_idx + 2],
        }) catch return null;
    }

    else if (hex_len == 6) {
        _ = fmt.hexToBytes(&byte_buf, str[start_idx..]) catch return null;
    }

    return Rgb {
        .r = byte_buf[0],
        .g = byte_buf[1],
        .b = byte_buf[2],
    };
}
