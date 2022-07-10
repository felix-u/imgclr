#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// EDIT PALETTE HERE
// default is solarised
int paletteLen = 18;
char* inputPalette[18] = {
    "93a1a1", "002b36",
    "073642", "224750",
    "dc322f", "dc322f",
    "859900", "859900",
    "b58900", "b58900",
    "268bd2", "268bd2",
    "6c71c4", "6c71c4",
    "2aa198", "2aa198",
    "657b83", "839496",
};

int badInput();

int main(int argc, char **argv) {

    char *inputFile = NULL;
    char *outputFile = NULL;
    int debug = 0, quiet = 0, slow = 0;

    // command line argument parsing
    int opt;
    while ((opt = getopt(argc, argv, "di:o:qs")) != -1) {
        switch (opt) {
            case 'd':
                debug = 1;
                // fall through
            case 'i':
                inputFile = optarg;
                break;
            case 'o':
                outputFile = optarg;
                break;
            case 'q':
                quiet = 1;
                // fall through
                break;
            case 's':
                slow = 1;
                // fall through
            default:
                break;
        }
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
    int whitespaceCounter = 0; int heightByteCounter = 0; int widthByteCounter = 0;
    int heightByteStart = 0; int widthByteStart = 0;
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
    for (int i = 0; i <= widthByteCounter; i++) {
        INPUT_WIDTH[i] = inputBytes[i + widthByteStart];
    }

    char INPUT_HEIGHT[heightByteCounter];
    for (int i = 0; i <= heightByteCounter; i++) {
        INPUT_HEIGHT[i] = inputBytes[i + heightByteStart];
    }


    // copy to output file, pixel by pixel
    const int WIDTH = atoi(INPUT_WIDTH); // use same width as input
    const int HEIGHT = atoi(INPUT_HEIGHT); // use same height as input
    if (debug) {
	printf("WIDTH: %d\n", WIDTH);
	printf("HEIGHT: %d\n", HEIGHT);
    }

    // write PPM header
    FILE *F_OUTPUT = fopen(outputFile, "wb");
    fprintf(F_OUTPUT, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
    if (debug) printf("Wrote header\n");

    unsigned char colour[3];
    const int byteOffset = 9 + widthByteCounter + heightByteCounter;
    /* P6 + newline + width + space + height + newline + 255 + newline
       2 + 1 + widthByteCounter + 1 + heightByteCounter + 1 + 3 + 1 */

    // ------------------------------------------
    // GET PALETTE IN DECIMAL FOR MATHEMATICAL COMPARISON WITH IMAGE
    // make array for storing palette in decimals
    int decimalPalette[paletteLen][3];
    if (debug) printf("Set palette of length %d\n", paletteLen);

    // iterate through characters of colours in input palette
    for (int i = 0; i < paletteLen; i++) {
        char *currentClr = inputPalette[i];
        unsigned int r, g, b;
        sscanf(currentClr, "%02x%02x%02x", &r, &g, &b);

        // store in decimalPalette
        decimalPalette[i][0] = r;
        decimalPalette[i][1] = g;
        decimalPalette[i][2] = b;
    }
    if (debug) printf("Converted palette to decimal format\n\n");
    // ------------------------------------------

    // setup to save previous match for a sort of "run-length" matching
    // so if current pixel = prev pixel, no need to recompute best match
    int prevR = 300, prevG = 300, prevB = 300;

    int bestMatch = 0;
    // write pixels to output
    if (!quiet)
	printf("Writing %dx%d image to %s\n", WIDTH, HEIGHT, outputFile);
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {

            if (debug) printf("Pos %d, %d\n", x+1, y+1);

            // current pixel position
            int currentPixelByte = 3*(x+WIDTH*y) + byteOffset;
            if (debug) printf("Byte %d\n", currentPixelByte);

            // get input colour values
            int inputR = inputBytes[currentPixelByte + 0];
            int inputG = inputBytes[currentPixelByte + 1];
            int inputB = inputBytes[currentPixelByte + 2];

            // FIXME - temp fix for negative RGB values
            // (idfk but this solves it. some kind of overflow)
            if (inputR < 0) { inputR = 255 + inputR; }
            if (inputG < 0) { inputG = 255 + inputG; }
            if (inputB < 0) { inputB = 255 + inputB; }

	    if (debug) printf("At %d,%d,%d\n", inputR, inputG, inputB);

	    // compare to prev pixel
	    if (inputR == prevR && inputG == prevG && inputB == prevB && !slow) {
		if (debug) printf("Reusing match from previous pixel\n");
		// this will go ahead to writing the pixel, below the else block
		// since we already have a good match
	    }
	    else { // otherwise, we need to compute a brand new match
		int comparisons[paletteLen];
		for (int i = 0; i < paletteLen; i++) { // channel differences
		    int diffR = decimalPalette[i][0] - inputR;
		    if (diffR < 0) { diffR = -diffR; }

		    int diffG = decimalPalette[i][1] - inputG;
		    if (diffG < 0) { diffG = -diffG; }

		    int diffB = decimalPalette[i][2] - inputB;
		    if (diffB < 0) { diffB = -diffB; }

		    comparisons[i] = diffR + diffG + diffB;
		    if (debug) printf("Comparison with colour %d is %d\n",
				      i, comparisons[i]);
		}
		// find best match out of all comparisons
		int diffTracker = 999; // impossibly large difference to start
		for (int i = 0; i < paletteLen; i++) {
		    if (comparisons[i] < diffTracker) {
			diffTracker = comparisons[i];
			bestMatch = i;
		    }
		}

		if (debug) printf("Best: index %d, value %d,%d,%d\n",
				  bestMatch,
				  decimalPalette[bestMatch][0],
				  decimalPalette[bestMatch][1],
				  decimalPalette[bestMatch][2]);
	    }

            // write pixel to output image
            colour[0] = decimalPalette[bestMatch][0]; // R
            colour[1] = decimalPalette[bestMatch][1]; // G
            colour[2] = decimalPalette[bestMatch][2]; // B

	    // save current pixels to prev variables, for next iteration
	    prevR = inputR; prevG = inputG; prevB = inputB;

	    if (debug) printf("Wrote %d,%d,%d\n\n",
			      colour[0], colour[1], colour[2]);

            fwrite(colour, 1, 3, F_OUTPUT);

        }

        // progress bar, unless quiet flag
        if (!quiet) {
            float prog = y;
            printf("\rProgress: [%3.0f%%] [", (prog/HEIGHT)*100);
            int barLen = 20;
            int count = (prog/HEIGHT)*barLen;
            for (int p = 0; p <= count; p++) putchar ('#');
            for (int p = 1; p < barLen-count; p++) putchar ('.');
            putchar(']');
            fflush(stdout);
        }
    }

    if (!quiet) putchar('\n');
    free(inputBytes);
    fclose(F_INPUT); fclose(F_OUTPUT);
    return 0;
}


// return if incorrect usage
int badInput(char *errorType) {
    printf("Error: ");
    switch(*errorType) {
        case 'a':
            printf("Not enough arguments");
        break; case 'h':
            printf("Hex values must be at least six characters in length.");
        break; case 'j' :
            printf("Please convert JPG to PPM");
        break; case 'p' :
            printf("Input not a PPM image");
        break; case 'n' :
            printf("Input file nonexistent");
        break; default:
            printf("Incorrect usage");
    }
    putchar('\n');
    return 1;
}