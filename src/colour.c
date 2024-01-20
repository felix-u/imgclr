typedef struct { u8 r, g, b; } Rgb;

static bool is_hex_char(char c) {
    if ((c >= 'a' && c <= 'f') ||
        (c >= 'A' && c <= 'A') ||
        (c >= '0' && c <= '9'))
    {
        return true;
    }
    return false;
}

static bool is_hex_cstr(char *str) {
    usize len = strlen(str);
    for (usize i = 0; i < len; i += 1) {
        if (!is_hex_char(str[i])) return false;
    }
    return true;
}

static error rgb_from_hex_str8(Str8 s, Rgb *out) {
    usize start_pos = 0;
    usize hex_len = 0;
    for (usize i = 0; i < s.len; i += 1) {
        if (!is_hex_char(s.ptr[i])) continue;
        start_pos = i;
        hex_len = s.len - start_pos;
        // Hex colour must be in either three-digit format (e.g. "fff") 
        // or six-digit format (e.g. "ffffff").
        if (hex_len != 3 && hex_len != 6) return errf(
            "hex colour '%.*s' must be of length 3 or 6", 
            str8_fmt(s)
        );
        if (!is_hex_cstr((char *)s.ptr + i)) {
            return errf("invalid hex colour '%.*s'", str8_fmt(s));
        }
        break;
    }

    // Now we've got a valid hex string to parse.
    if (hex_len == 3) {
        char r_as_str[] = { s.ptr[start_pos + 0], '\0' };
        char g_as_str[] = { s.ptr[start_pos + 1], '\0' };
        char b_as_str[] = { s.ptr[start_pos + 2], '\0' };
        u8 single_r = (u8)strtol(r_as_str, NULL, 16);
        u8 single_g = (u8)strtol(g_as_str, NULL, 16);
        u8 single_b = (u8)strtol(b_as_str, NULL, 16);
        out->r = single_r * 16 + single_r;
        out->g = single_g * 16 + single_g;
        out->b = single_b * 16 + single_b;
        return 0;
    } else if (hex_len == 6) {
        char r1_as_str[] = { s.ptr[start_pos + 0], '\0' };
        char r2_as_str[] = { s.ptr[start_pos + 1], '\0' };
        char g1_as_str[] = { s.ptr[start_pos + 2], '\0' };
        char g2_as_str[] = { s.ptr[start_pos + 3], '\0' };
        char b1_as_str[] = { s.ptr[start_pos + 4], '\0' };
        char b2_as_str[] = { s.ptr[start_pos + 5], '\0' };
        u8 single_r1 = (u8)strtol(r1_as_str, NULL, 16);
        u8 single_r2 = (u8)strtol(r2_as_str, NULL, 16);
        u8 single_g1 = (u8)strtol(g1_as_str, NULL, 16);
        u8 single_g2 = (u8)strtol(g2_as_str, NULL, 16);
        u8 single_b1 = (u8)strtol(b1_as_str, NULL, 16);
        u8 single_b2 = (u8)strtol(b2_as_str, NULL, 16);
        out->r = single_r1 * 16 + single_r2;
        out->g = single_g1 * 16 + single_g2;
        out->b = single_b1 * 16 + single_b2;
        return 0;
    }

    return errf("invalid hex colour '%.*s'", str8_fmt(s));
}
