typedef struct {
    int x; // x offset
    int y; // y offset
    float ratio; // what to multiply error by
} ErrorSet;

typedef struct {
    char *name;
    int len;
    ErrorSet offsets[];
} Algorithm;


// Default
// https://en.wikipedia.org/wiki/Floyd–Steinberg_dithering
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

#define NUM_OF_ALGORITHMS 3

// @Missing The other algorithms @Missing

const Algorithm *ALGORITHMS[NUM_OF_ALGORITHMS] = {
    &floyd_steinberg,
    &none,
    &atkinson
};
