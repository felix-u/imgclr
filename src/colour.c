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


static Rgb *clr_hexToRGB(char *str, Rgb *rgb) {

    usize start_pos = 0;
    usize hex_len = 0;

    // Find hex format & ignore starting characters
    // (anything which isn't a-fA-F0-9).
    usize len = strlen(str);
    for (usize i = 0; i < len; i += 1) {
        if (!is_hex_char(str[i])) continue;
        start_pos = i;
        hex_len = strlen(str) - start_pos;
        // Hex colour must be in either three-digit format (e.g. "fff") 
        // or six-digit format (e.g. "ffffff").
        if (hex_len != 3 && hex_len != 6) return NULL;
        if (!is_hex_cstr(str + i)) return NULL;
        break;
    }


    // Now we've got a valid hex string to parse.
    if (hex_len == 3) {
        char r_as_str[] = { str[start_pos + 0], '\0' };
        char g_as_str[] = { str[start_pos + 1], '\0' };
        char b_as_str[] = { str[start_pos + 2], '\0' };
        u8 single_r = (u8)strtol(r_as_str, NULL, 16);
        u8 single_g = (u8)strtol(g_as_str, NULL, 16);
        u8 single_b = (u8)strtol(b_as_str, NULL, 16);
        rgb->r = single_r * 16 + single_r;
        rgb->g = single_g * 16 + single_g;
        rgb->b = single_b * 16 + single_b;
        return rgb;
    }
    else if (hex_len == 6) {
        char r1_as_str[] = { str[start_pos + 0], '\0' };
        char r2_as_str[] = { str[start_pos + 1], '\0' };
        char g1_as_str[] = { str[start_pos + 2], '\0' };
        char g2_as_str[] = { str[start_pos + 3], '\0' };
        char b1_as_str[] = { str[start_pos + 4], '\0' };
        char b2_as_str[] = { str[start_pos + 5], '\0' };
        u8 single_r1 = (u8)strtol(r1_as_str, NULL, 16);
        u8 single_r2 = (u8)strtol(r2_as_str, NULL, 16);
        u8 single_g1 = (u8)strtol(g1_as_str, NULL, 16);
        u8 single_g2 = (u8)strtol(g2_as_str, NULL, 16);
        u8 single_b1 = (u8)strtol(b1_as_str, NULL, 16);
        u8 single_b2 = (u8)strtol(b2_as_str, NULL, 16);
        rgb->r = single_r1 * 16 + single_r2;
        rgb->g = single_g1 * 16 + single_g2;
        rgb->b = single_b1 * 16 + single_b2;
        return rgb;
    }

    // If we've not returned by now, there's something wrong.
    return NULL;
}
