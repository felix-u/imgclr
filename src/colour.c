#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    bool valid;
} RGBCheck;

bool isValidHexChar(char c) {
    if ((c >= 'a' && c <= 'f') ||
        (c >= 'A' && c <= 'A') ||
        (c >= '0' && c <= '9'))
    {
        return true;
    }

    return false;
}

bool strHasValidHexChars(char *str) {
    for (size_t i = 0; i < strlen(str); i++) {
        if (!isValidHexChar(str[i])) {
            return false;
        }
    }
    return true;
}


RGBCheck hexStrToRGB(char *str) {

    int start_pos = 0;
    int hex_len = 0;

    // Work out format of hex string and ignore starting characters, such as a
    // hash symbol or anything else which isn't a-fA-F0-9.

    for (size_t i = 0; i < strlen(str); i++) {

        if (isValidHexChar(str[i])) {

            start_pos = i;
            hex_len = strlen(str) - start_pos;

            // Hex colour must be in either three-digit format (e.g. "fff") or
            // six-digit format (e.g. "ffffff").

            if (hex_len == 3 || hex_len == 6) {
                // Correct length but invalid chars
                if (!strHasValidHexChars(str + i)) {
                    RGBCheck rgb_return = {1, 1, 1, false};
                    return rgb_return;
                }
            }
            // Invalid length
            else {
                RGBCheck rgb_return = {2, 2, 2, false};
                return rgb_return;
            }

            break;
        }
    }

    // Now we've got a valid hex string to parse.
    if (hex_len == 3) {
        char r_as_str[] = { str[start_pos + 0], '\0' };
        char g_as_str[] = { str[start_pos + 1], '\0' };
        char b_as_str[] = { str[start_pos + 2], '\0' };
        uint8_t single_r = strtol(r_as_str, NULL, 16);
        uint8_t single_g = strtol(g_as_str, NULL, 16);
        uint8_t single_b = strtol(b_as_str, NULL, 16);
        uint8_t r = single_r * 16 + single_r;
        uint8_t g = single_g * 16 + single_g;
        uint8_t b = single_b * 16 + single_b;
        RGBCheck rgb_return = {r, g, b, true};
        return rgb_return;
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
        uint8_t r = single_r1 * 16 + single_r2;
        uint8_t g = single_g1 * 16 + single_g2;
        uint8_t b = single_b1 * 16 + single_b2;
        RGBCheck rgb_return = {r, g, b, true};
        return rgb_return;
    }

    // If we've not returned by now, there's something wrong
    RGBCheck rgb_return = {0, 0, 0, false};
    return rgb_return;
}
