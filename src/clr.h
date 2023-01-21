#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef CLR_TYPE
#define CLR_TYPE

typedef struct clr_RGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} clr_RGB;

#endif // CLR_TYPE


bool clr_charIsHex(char c);
bool clr_strIsHex(char *str);
clr_RGB *clr_hexToRGB(char *str, clr_RGB *rgb);


#ifdef CLR_IMPLEMENTATION

bool clr_charIsHex(char c) {
    if ((c >= 'a' && c <= 'f') ||
        (c >= 'A' && c <= 'A') ||
        (c >= '0' && c <= '9'))
    {
        return true;
    }
    return false;
}


bool clr_strIsHex(char *str) {
    for (size_t i = 0; i < strlen(str); i++) {
        if (!clr_charIsHex(str[i])) return false;
    }
    return true;
}


clr_RGB *clr_hexToRGB(char *str, clr_RGB *rgb) {

    size_t start_pos = 0;
    size_t hex_len = 0;

    // Find hex format & ignore starting characters - anything which isn't a-fA-F0-9.
    for (size_t i = 0; i < strlen(str); i++) {
        if (!clr_charIsHex(str[i])) continue;
        start_pos = i;
        hex_len = strlen(str) - start_pos;
        // Hex colour must be in either three-digit format (e.g. "fff") or six-digit format (e.g. "ffffff").
        if (hex_len != 3 && hex_len != 6) return NULL;
        if (!clr_strIsHex(str + i)) return NULL;
        break;
    }


    // Now we've got a valid hex string to parse.
    if (hex_len == 3) {
        char r_as_str[] = { str[start_pos + 0], '\0' };
        char g_as_str[] = { str[start_pos + 1], '\0' };
        char b_as_str[] = { str[start_pos + 2], '\0' };
        uint8_t single_r = strtol(r_as_str, NULL, 16);
        uint8_t single_g = strtol(g_as_str, NULL, 16);
        uint8_t single_b = strtol(b_as_str, NULL, 16);
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
        uint8_t single_r1 = strtol(r1_as_str, NULL, 16);
        uint8_t single_r2 = strtol(r2_as_str, NULL, 16);
        uint8_t single_g1 = strtol(g1_as_str, NULL, 16);
        uint8_t single_g2 = strtol(g2_as_str, NULL, 16);
        uint8_t single_b1 = strtol(b1_as_str, NULL, 16);
        uint8_t single_b2 = strtol(b2_as_str, NULL, 16);
        rgb->r = single_r1 * 16 + single_r2;
        rgb->g = single_g1 * 16 + single_g2;
        rgb->b = single_b1 * 16 + single_b2;
        return rgb;
    }

    // If we've not returned by now, there's something wrong.
    return NULL;
}

#endif // CLR_IMPLEMENTATION
