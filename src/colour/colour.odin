package colour

import "core:strconv"

RGBA :: struct { r, g, b, a: u8 }


hexStrToRGBA :: proc(str: string) -> (RGBA, bool) {

    pos := 0;
    hex_len: int;

    // Work out format of hex string and ignore starting characters, such as a
    // hash symbol or anything else which isn't a-fA-F0-9.

    for char, index in str {
        if isValidHexChar(char) {

            pos = index;
            hex_len = len(str) - pos;

            // Hex colour must be in either three-digit format (e.g. "fff") or
            // six-digit format (e.g. "ffffff").

            if hex_len == 3 || hex_len == 6 {
                // Correct length but invalid chars
                if !strHasValidHexChars(str[pos : ]) {
                    return RGBA{}, false;
                }
            }
            else do return RGBA{}, false; // Invalid length
            break;
        }
    }

    // Now we've got a valid hex string to parse.
    if hex_len == 3 {
        single_r, _ := strconv.parse_u64_of_base(str[pos + 0 : pos + 1], 16);
        single_g, _ := strconv.parse_u64_of_base(str[pos + 1 : pos + 2], 16);
        single_b, _ := strconv.parse_u64_of_base(str[pos + 2 : pos + 3], 16);
        r := u8(single_r * 16 + single_r);
        g := u8(single_g * 16 + single_g);
        b := u8(single_b * 16 + single_b);
        return RGBA{r, g, b, 255}, true;
    }
    else if hex_len == 6 {
        r, _ := strconv.parse_u64_of_base(str[pos + 0: pos + 2], 16);
        g, _ := strconv.parse_u64_of_base(str[pos + 2: pos + 4], 16);
        b, _ := strconv.parse_u64_of_base(str[pos + 4: pos + 6], 16);
        return RGBA{u8(r), u8(g), u8(b), 255}, true;
    }

    return RGBA{}, false;
}


isValidHexChar :: proc(char: rune) -> bool {
    switch char {
    case 'a'..='f', 'A'..='F', '0'..='9':
        return true;
    case:
        return false;
    }
}


strHasValidHexChars :: proc(str: string) -> bool {
    for char in str {
        if !isValidHexChar(char) do return false;
    }
    return true;
}
