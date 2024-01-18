#define DITHER_ALGORITHM_NUM  6
#define DITHER_MAX_OFFSET_NUM 12

typedef struct {
    i8 x_offset;
    i8 y_offset;
    f64 factor;
} Dither_Error;

typedef struct {
    Str8 name;
    usize offset_num;
    Dither_Error offsets[DITHER_MAX_OFFSET_NUM];
} Dither_Algorithm;

// https://en.wikipedia.org/wiki/Floyd–Steinberg_dithering
//   * 7
// 3 5 1
// multiplier: 1/16
const Dither_Algorithm floyd_steinberg = {
    .name = str8("floyd-steinberg"),
    .offset_num = 4,
    .offsets = {
        { 1, 0, 7.0/16.0},
        {-1, 1, 3.0/16.0},
        { 0, 1, 5.0/16.0},
        { 1, 1, 1.0/16.0},
    },
};


const Dither_Algorithm none = {
    .name = str8("none"),
    .offset_num = 0,
    .offsets = {{0, 0, 0}},
};


// https://gaz.github.io/canvas-atkinson-dither
//   * 1 1
// 1 1 1
//   1
// multiplier: 1/8
const Dither_Algorithm atkinson = {
    .name = str8("atkinson"),
    .offset_num = 6,
    .offsets = {
        { 1, 0, 1.0/8.0},
        { 2, 0, 1.0/8.0},
        {-1, 1, 1.0/8.0},
        { 0, 1, 1.0/8.0},
        { 1, 1, 1.0/8.0},
        { 1, 2, 1.0/8.0},
    },
};


// https://en.wikipedia.org/wiki/Error_diffusion
//     * 7 5
// 3 5 7 5 3
// 1 3 5 3 1
// multiplier = 1/48
const Dither_Algorithm jjn = {
    .name = str8("jjn"),
    .offset_num = 12,
    .offsets = {
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
        { 2, 2, 1.0/48.0},
    },
};


// https://tannerhelland.com/2012/12/28/dithering-eleven-algorithms-source-code.html
//     * 8 4
// 2 4 8 4 2
// multiplier = 1/32
const Dither_Algorithm burkes = {
    .name = str8("burkes"),
    .offset_num = 7,
    .offsets = {
        { 1, 0, 8.0/32.0},
        { 2, 0, 4.0/32.0},
        {-2, 1, 2.0/32.0},
        {-1, 1, 4.0/32.0},
        { 0, 1, 8.0/32.0},
        { 1, 1, 4.0/32.0},
        { 2, 1, 2.0/32.0},
    },
};


// Results are VERY similar to Floyd-Steinberg, and execution speed is slightly faster.
const Dither_Algorithm sierra_lite = {
    .name = str8("sierra-lite"),
    .offset_num = 3,
    .offsets = {
        { 1, 0, 2.0/4.0},
        {-1, 1, 1.0/4.0},
        { 0, 1, 1.0/4.0},
    },
};

const Dither_Algorithm *DITHER_ALGORITHMS[DITHER_ALGORITHM_NUM] = {
    &floyd_steinberg,
    &none,
    &atkinson,
    &jjn,
    &burkes,
    &sierra_lite,
};
