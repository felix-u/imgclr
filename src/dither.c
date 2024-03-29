typedef struct {
    i8 x_offset;
    i8 y_offset;
    f64 factor;
} Dither_Error;

typedef Slice(Dither_Error) Dither_Algorithm;

// https://en.wikipedia.org/wiki/Floyd–Steinberg_dithering
//   * 7
// 3 5 1
// multiplier: 1/16
Dither_Error floyd_steinberg_offsets[] = {
    { 1, 0, 7.0/16.0},
    {-1, 1, 3.0/16.0},
    { 0, 1, 5.0/16.0},
    { 1, 1, 1.0/16.0},
};
const Dither_Algorithm floyd_steinberg = slice(floyd_steinberg_offsets);

const Dither_Algorithm none = {0};

// https://gaz.github.io/canvas-atkinson-dither
//   * 1 1
// 1 1 1
//   1
// multiplier: 1/8
Dither_Error atkinson_offsets[] = {
    { 1, 0, 1.0/8.0},
    { 2, 0, 1.0/8.0},
    {-1, 1, 1.0/8.0},
    { 0, 1, 1.0/8.0},
    { 1, 1, 1.0/8.0},
    { 1, 2, 1.0/8.0},
};
const Dither_Algorithm atkinson = slice(atkinson_offsets);

// https://en.wikipedia.org/wiki/Error_diffusion
//     * 7 5
// 3 5 7 5 3
// 1 3 5 3 1
// multiplier = 1/48
Dither_Error jjn_offsets[] = {
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
};
const Dither_Algorithm jjn = slice(jjn_offsets);

// https://tannerhelland.com/2012/12/28/dithering-eleven-algorithms-source-code.html
//     * 8 4
// 2 4 8 4 2
// multiplier = 1/32
Dither_Error burkes_offsets[] = {
    { 1, 0, 8.0/32.0},
    { 2, 0, 4.0/32.0},
    {-2, 1, 2.0/32.0},
    {-1, 1, 4.0/32.0},
    { 0, 1, 8.0/32.0},
    { 1, 1, 4.0/32.0},
    { 2, 1, 2.0/32.0},
};
const Dither_Algorithm burkes = slice(burkes_offsets);

// Results are VERY similar to Floyd-Steinberg, and execution speed is slightly
// faster.
Dither_Error sierra_lite_offsets[] = {
    { 1, 0, 2.0/4.0},
    {-1, 1, 1.0/4.0},
    { 0, 1, 1.0/4.0},
};
const Dither_Algorithm sierra_lite = slice(sierra_lite_offsets);
