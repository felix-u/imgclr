#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int badInput();

int main(int argc, char **argv) {

    char *inputFile = NULL;
    char *outputFile = NULL;

    // command line argument parsing
    int c;
    while ((c = getopt(argc, argv, "i:o:")) != -1)
        switch (c) {
            case 'i':
                inputFile = optarg;
                break;
            case 'o':
                outputFile = optarg;
                break;
            default:
                break;
        }

    // fail with wrong usage message if mandatory options missing
    if (inputFile == NULL || outputFile == NULL) {
        return badInput("a");
    }

    FILE *F_INPUT = fopen(inputFile, "r");
    // check that input file exists and exit if not
    if (F_INPUT == NULL) {
        return badInput("n");
    }

    // read bytes to check if JPEG - error if so
    unsigned char bytes[3];
    fread(bytes, 3, 1, F_INPUT);
    if (bytes[0] == 255 && bytes[1] == 216 && bytes[2] == 255) {
        return badInput("j");
    }
    // check if PPM - error if not
    if (bytes[0] != 80 /*P*/ || bytes[1] != 54 /*6*/) {
        return badInput("p");
    }

    // save bytes of input to array inputBytes
    fseek(F_INPUT, 0, SEEK_END); // jump to end of file
    long inputLen = ftell(F_INPUT); // get current byte offset
    rewind(F_INPUT); // go back to beginning of file
    // enough memory for the file
    char *inputBytes = (char *)malloc(inputLen * sizeof(char));
    fread(inputBytes, inputLen, 1, F_INPUT); // read in file

    /* ASCII: 10 = newline, 32 = space
    increment counter so we know to get WIDTH (after newline, before space), or
    HEIGHT (after space, before newline). we also need to know how many bytes
    are in WIDTH and HEIGHT */
    int whitespaceCounter = 0, heightByteCounter = 0, widthByteCounter = 0;
    int heightByteStart = 0, widthByteStart;
    while (whitespaceCounter < 5) {
        for (int i= 0; i < inputLen; i++) {
            if (inputBytes[i] == 10 || inputBytes[i] == 32) {
                whitespaceCounter++;
            }

            if (inputBytes[i] != 10 && inputBytes[i] != 32) {
                switch(whitespaceCounter) {
                    case 1:
                        widthByteStart = i;
                        whitespaceCounter++;
                        widthByteCounter++; break;
                    case 2:
                        widthByteCounter++; break;
                    case 3:
                        heightByteStart = i;
                        whitespaceCounter++;
                        heightByteCounter++; break;
                    case 4:
                        heightByteCounter++; break;
                }
            }
        }
    }

    /* we now know how many bytes WIDTH and HEIGHT take up, and at which byte
    they start */
    char INPUT_WIDTH[widthByteCounter];
    for (int i = 0; i < widthByteCounter; i++) {
        INPUT_WIDTH[i] = inputBytes[i + widthByteStart];
    }

    char INPUT_HEIGHT[heightByteCounter];
    for (int i = 0; i < heightByteCounter; i++) {
        INPUT_HEIGHT[i] = inputBytes[i + heightByteStart];
    }


    // copy to output file, pixel by pixel
    const int WIDTH = atoi(INPUT_WIDTH); // use same width as input
    const int HEIGHT = atoi(INPUT_HEIGHT); // use same height as input
    printf("WIDTH: %d\n", WIDTH);
    printf("HEIGHT: %d\n", HEIGHT);

    // write PPM header
    FILE *F_OUTPUT = fopen(outputFile, "wb");
    fprintf(F_OUTPUT, "P6\n%d %d\n255\n", WIDTH, HEIGHT);

    unsigned char colour[3];
    const int byteOffset = 9 + widthByteCounter + heightByteCounter;
            /* P6 + newline + width + space + height + newline + 255 + newline
               2 + 1 + widthByteCounter + 1 + heightByteCounter + 1 + 3 + 1 */

    // TODO - get palette from command line args or input file, don't hardcode
    // placeholder colour palette

    // solarised
    char *inputPalette[18] = {"93a1a1", "002b36",
                              "073642", "224750",
                              "dc322f", "E35D5B",
                              "859900", "B1CC00",
                              "b58900", "e8b000",
                              "268bd2", "4CA2DF",
                              "6c71c4", "9094D3",
                              "2aa198", "35C9BE",
                              "657b83", "839496",
                             };

    // neutral test palette
    // char *inputPalette[18] = {"1c1c1e", "f5f5f7",
    //                           "2c2c2e", "3a3a3c",
    //                           "E8322F", "ed5f5d",
    //                           "619942", "79b757",
    //                           "F0A81B", "f3ba4b",
    //                           "2072F4", "5191f6",
    //                           "DE3281", "e55e9c",
    //                           "2AB2CA", "4ec5da",
    //                           "8e8e93", "d1d1d6",
    //                          };

    // TODO - calculate number of colours in palette, don't hardcode
    int paletteLen = 18;


    // ------------------------------------------
    // GET PALETTE IN DECIMAL FOR MATHEMATICAL COMPARISON WITH IMAGE
    // make array for storing palette in decimals
    int decimalPalette[sizeof(inputPalette)/sizeof(inputPalette[0])][3];

    // iterate through characters of colours in input palette
    for (long unsigned int i = 0;
            i < sizeof(inputPalette)/sizeof(inputPalette[0]); i++) {
        char *currentClr = inputPalette[i];
        int r, g, b;
        sscanf(currentClr, "%02x%02x%02x", &r, &g, &b);

        // store in decimalPalette
        decimalPalette[i][0] = r;
        decimalPalette[i][1] = g;
        decimalPalette[i][2] = b;
    }
    // ------------------------------------------

    // TODO
    // setup for storing existing matches to use them where possible rather
    // than recalculating
    // int matchedPixels[WIDTH*HEIGHT];


    // write pixels to output
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {

            // current pixel position
            int currentPixelByte = 3*(x+WIDTH*y) + byteOffset;

            // get input colour values
            int inputR = inputBytes[currentPixelByte + 0];
            int inputG = inputBytes[currentPixelByte + 1];
            int inputB = inputBytes[currentPixelByte + 2];

            // FIXME - temp fix for negative RGB values
            // (idfk but this solves it. maybe should use long int?)
            if (inputR < 0) { inputR = 255 + inputR; }
            if (inputG < 0) { inputG = 255 + inputG; }
            if (inputB < 0) { inputB = 255 + inputB; }

            // find closest match for pixel in palette
            int comparisons[paletteLen];
            for (int i = 0; i < paletteLen; i++) { // channel differences
                int diffR = decimalPalette[i][0] - inputR;
                if (diffR < 0) { diffR = -diffR; }

                int diffG = decimalPalette[i][1] - inputG;
                if (diffG < 0) { diffG = -diffG; }

                int diffB = decimalPalette[i][2] - inputB;
                if (diffB < 0) { diffB = -diffB; }

                comparisons[i] = diffR + diffG + diffB;
            }
            // find best match out of all comparisons
            int bestMatch = 0;
            int diffTracker = 999; // impossibly large difference to start
            for (int i = 0; i < paletteLen; i++) {
                if (comparisons[i] < diffTracker) {
                    diffTracker = comparisons[i-1];
                    bestMatch = i-1;
                }
            }

            // write pixel to output image

            /* // CASE - copy original without converting
            colour[0] = inputR; // R
            colour[1] = inputG; // G
            colour[2] = inputB; // B
            */

            // CASE - copy with new palette
            colour[0] = decimalPalette[bestMatch][0]; // R
            colour[1] = decimalPalette[bestMatch][1]; // G
            colour[2] = decimalPalette[bestMatch][2]; // B

            fwrite(colour, 1, 3, F_OUTPUT);

        }
    }

    fclose(F_INPUT); fclose(F_OUTPUT);
    return 0;
}


// return if incorrect usage
int badInput(char *errorType) {
    switch(*errorType) {
        case 'a':
            printf("Not enough arguments \n");
        break; case 'j' :
            printf("Please convert JPG to PPM \n");
        break; case 'p' :
            printf("Input not a PPM image \n");
        break; case 'n' :
            printf("Input file nonexistent \n");
        break; default:
            printf("Incorrect usage \n");
    }
    return 1;
}
