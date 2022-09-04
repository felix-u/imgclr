#include <stdint.h>

#define MAX_OFFSETS 12
#define NUM_OF_ALGORITHMS 6

typedef struct {
    int x; // x offset
    int y; // y offset
    float ratio; // what to multiply error by
} ErrorSet;

typedef struct {
    char name[32];
    int offset_num;
    ErrorSet offsets[MAX_OFFSETS];
} Algorithm;


// Default
// https://en.wikipedia.org/wiki/Floyd–Steinberg_dithering
//   * 7
// 3 5 1
// multiplier: 1/16

const Algorithm floyd_steinberg = {
    "floyd-steinberg",
    4,
    {
        { 1, 0, 7.0/16.0},
        {-1, 1, 3.0/16.0},
        { 0, 1, 5.0/16.0},
        { 1, 1, 1.0/16.0},
    }
};


// Used to disable dithering without having to add more logic when iterating
// over every pixel

const Algorithm none = {
    "none",
    0,
    {{0, 0, 0}}
};


// https://gaz.github.io/canvas-atkinson-dither
//   * 1 1
// 1 1 1
//   1
// multiplier: 1/8

const Algorithm atkinson = {
    "atkinson",
    6,
    {
        { 1, 0, 1.0/8.0},
        { 2, 0, 1.0/8.0},
        {-1, 1, 1.0/8.0},
        { 0, 1, 1.0/8.0},
        { 1, 1, 1.0/8.0},
        { 1, 2, 1.0/8.0}
    }
};


// https://en.wikipedia.org/wiki/Error_diffusion
//     * 7 5
// 3 5 7 5 3
// 1 3 5 3 1
// multiplier = 1/48

const Algorithm jjn = {
    "jjn",
    12,
    {
        { 1, 0, 7.0/48.0},
        { 2, 0, 5.0/48.0},
        {-2, 1, 3.0/48.0},
        {-1, 1, 5.0/48.0},
        { 0, 1, 7.0/48.0},
        { 1, 1, 5.0/48.0},
        { 2, 1, 3.0/48.0},
        {-2, 2, 1.0/48.0},
        {-1, 2, 3.0/48.0},
        { 0, 2, 5.0/48.0},
        { 1, 2, 3.0/48.0},
        { 2, 2, 1.0/48.0}
    }
};


// https://tannerhelland.com/2012/12/28/dithering-eleven-algorithms-source-code.html
//     * 8 4
// 2 4 8 4 2
// multiplier = 1/32

const Algorithm burkes = {
    "burkes",
    7,
    {
        { 1, 0, 8.0/32.0},
        { 2, 0, 4.0/32.0},
        {-2, 1, 2.0/32.0},
        {-1, 1, 4.0/32.0},
        { 0, 1, 8.0/32.0},
        { 1, 1, 4.0/32.0},
        { 2, 1, 2.0/32.0}
    }
};


// Perhaps this should be default? Results are VERY similar to
// Floyd-Steinberg, and execution speed is slightly faster.

const Algorithm sierra_lite = {
    "sierra-lite",
    3,
    {
        { 1, 0, 2.0/4.0},
        {-1, 1, 1.0/4.0},
        { 0, 1, 1.0/4.0}
    }
};


// @Feature Implement ordered dithering, maybe?
// https://en.wikipedia.org/wiki/Ordered_dithering @Feature


const Algorithm *ALGORITHMS[NUM_OF_ALGORITHMS] = {
    &floyd_steinberg,
    &none,
    &atkinson,
    &jjn,
    &burkes,
    &sierra_lite
};