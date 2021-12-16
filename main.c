#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int badInput();

int main(int argc, char **argv) {

    char *inputFile = NULL;
    char *outputFile = NULL;

    inputFile = argv[1];  // first argument is input file
    outputFile = argv[2]; // second argument is output file
    // TODO: handle cases where program itself is not first argument

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
    char *inputBytes = (char *)malloc(inputLen * sizeof(char)); // enough memory for the file
    fread(inputBytes, inputLen, 1, F_INPUT); // read in file

    /* /1* DEBUGGING *1/ */
    /* for (int i = 0; i < inputLen; i++) { */
    /*     printf("%d\n", inputBytes[i]); */
    /* } */


    /* ASCII: 10 = newline, 32 = space
    increment counter so we know to get WIDTH (after newline, before space), or
    HEIGHT (after space, before newline). we also need to know how many bytes
    are in WIDTH and HEIGHT */
    int whitespaceCounter = 0, heightByteCounter = 0, widthByteCounter = 0;
    int heightByteStart, widthByteStart;
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

    // placeholder colour palette
    char *inputPalette[9] = {"000000", "808080", "ffffff",
                             "ff8080", "80ff80", "8080ff",
                             "ffff80", "80ffff", "ff80ff"};

    // DEBUGGING - print colour palette
    printf("Palette as specified:");
    for (long unsigned int i = 0; i < sizeof(inputPalette)/sizeof(inputPalette[0]); i++) {
        printf(" %s", inputPalette[i]);
    }
    putchar(10);

    // get palette in decimal for mathematical comparison with image
    // make array for storing palette in decimals
    int decimalPalette[sizeof(inputPalette)/sizeof(inputPalette[0])][3] = {};

    // iterate through characters of colours in input palette
    for (long unsigned int i = 0; i < sizeof(inputPalette)/sizeof(inputPalette[0]); i++) {

    }

    // test conversion to RGB - WORKS
    char *background = inputPalette[0];
    printf("%s \n", background);
    int r, g, b;
    sscanf(background, "%02x%02x%02x", &r, &g, &b);
    printf("rgb(%d, %d, %d) \n", r, g, b);


    // write pixels to output
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int currentPixelByte = 3*(x+WIDTH*y);
            colour[0] = inputBytes[currentPixelByte + byteOffset + 0]; // R
            colour[1] = inputBytes[currentPixelByte + byteOffset + 1]; // G
            colour[2] = inputBytes[currentPixelByte + byteOffset + 2]; // B
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

/* /1* command line argument parsing *1/ */
/* int argIndex = 0; */
/* /1* begin implementing options - disabled for now *1/ */
/* while ((argIndex = getopt(argc, argv, "i:o")) != -1) { */
/*   switch (argIndex) { */
/*   case 'i': */
/*     inputFile = optarg + 1; */
/*     printf("inputFile is %s\n", inputFile); */
/*     break; */
/*   case 'o': */
/*     outputFile = optarg + 1; */
/*     printf("outputFile is %s\n", outputFile); */
/*     break; */
/*   default: */
/*     return badInput(); */
/*   } */
/* } */
