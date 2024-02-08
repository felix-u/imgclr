typedef struct { u8 r, g, b; } Rgb;

const bool is_hex_char_table[256] = {
    ['0'] = 1, ['1'] = 1, ['2'] = 1, ['3'] = 1, ['4'] = 1, 
    ['5'] = 1, ['6'] = 1, ['7'] = 1, ['8'] = 1, ['9'] = 1, 
    ['A'] = 1, ['B'] = 1, ['C'] = 1, ['D'] = 1, ['E'] = 1, ['F'] = 1,
    ['a'] = 1, ['b'] = 1, ['c'] = 1, ['d'] = 1, ['e'] = 1, ['f'] = 1,
};

static bool is_hex_str8(Str8 s) {
    for (usize i = 0; i < s.len; i += 1) {
        if (!is_hex_char_table[s.ptr[i]]) return false;
    }
    return true;
}

static error rgb_from_hex_str8(Str8 s, Rgb *out) {
    if (s.len != 3 && s.len != 6) return errf(
        "hex colour '%.*s' must be of length 3 or 6", 
        str8_fmt(s)
    );
    if (!is_hex_str8(s)) {
        return errf("invalid hex colour '%.*s'", str8_fmt(s));
    }

    if (s.len == 3) {
        usize value = decimal_from_hex_str8(s);
        u8 r = (value & 0xf00) >> 8;
        u8 g = (value & 0x0f0) >> 4;
        u8 b = (value & 0x00f);
        out->r = r * 16 + r;
        out->g = g * 16 + g;
        out->b = b * 16 + b;
    } else if (s.len == 6) {
        usize value = decimal_from_hex_str8(s);
        out->r = (value & 0xff0000) >> 16;
        out->g = (value & 0x00ff00) >> 8;
        out->b = (value & 0x0000ff);
    }
    return 0;
}
