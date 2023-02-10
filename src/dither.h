#include <stdint.h>
#include <stdlib.h>


#ifndef DITHER_TYPE
#define DITHER_TYPE

#define DITHER_ALGORITHM_NUM  6
#define DITHER_MAX_OFFSET_NUM 12

typedef struct dither_Error {
    const int8_t x;   // x offset
    const int8_t y;   // y offset
    const double ratio; // what to multiply error by
} dither_Error;

typedef struct dither_Algorithm {
    const char name[32];
    const size_t offset_num;
    const dither_Error offsets[12];
} dither_Algorithm;

#endif // DITHER_TYPE


#ifdef DITHER_IMPLEMENTATION

// https://en.wikipedia.org/wiki/Floyd–Steinberg_dithering
//   * 7
// 3 5 1
// multiplier: 1/16
const dither_Algorithm floyd_steinberg = {
    "floyd-steinberg",
    4,
    {
        { 1, 0, 7.0/16.0},
        {-1, 1, 3.0/16.0},
        { 0, 1, 5.0/16.0},
        { 1, 1, 1.0/16.0},
    }
};


// Used to disable dithering without having to add more logic to handle the disabled case.
const dither_Algorithm none = {
    "none",
    0,
    {{0, 0, 0}}
};


// https://gaz.github.io/canvas-atkinson-dither
//   * 1 1
// 1 1 1
//   1
// multiplier: 1/8
const dither_Algorithm atkinson = {
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
const dither_Algorithm jjn = {
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
const dither_Algorithm burkes = {
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


// Results are VERY similar to Floyd-Steinberg, and execution speed is slightly faster.
const dither_Algorithm sierra_lite = {
    "sierra-lite",
    3,
    {
        { 1, 0, 2.0/4.0},
        {-1, 1, 1.0/4.0},
        { 0, 1, 1.0/4.0}
    }
};

const dither_Algorithm *DITHER_ALGORITHMS[DITHER_ALGORITHM_NUM] = {
    &floyd_steinberg,
    &none,
    &atkinson,
    &jjn,
    &burkes,
    &sierra_lite
};

#endif // DITHER_IMPLEMENTATION
