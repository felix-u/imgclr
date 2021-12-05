#include <stdio.h>
#include <stdlib.h>
/* for command line argument parsing and a couple other things*/
#include <unistd.h>

int badInput();

int main(int argc, char **argv) {

    char *inputFile = NULL;
    char *outputFile = NULL;

    inputFile = argv[1];  /* first argument is input file */
    outputFile = argv[2]; /* second argument is output file */

    /* fail with wrong usage message if mandatory options missing */
    if (inputFile == NULL || outputFile == NULL) {
        return badInput("a");
    }

    FILE *F_INPUT = fopen(inputFile, "r");
    /* check that input file exists and exit if not*/
    if (F_INPUT == NULL) {
        return badInput("n");
    }

    /* read bytes to check if JPEG - error if so */
    unsigned char bytes[3];
    fread(bytes, 3, 1, F_INPUT);
    if (bytes[0] == 255 && bytes[1] == 216 && bytes[2] == 255) {
        return badInput("j");
    }
    /* check if PPM - error if not */
    if (bytes[0] != 80 /*P*/ || bytes[1] != 54 /*6*/) {
        return badInput("p");
    }


    /* save bytes of input to array inputBytes */
    fseek(F_INPUT, 0, SEEK_END); /* jump to end of file */
    long inputLen = ftell(F_INPUT); /* get current byte offset */
    rewind(F_INPUT); /* go back to beginning of file */
    char *inputBytes = (char *)malloc(inputLen * sizeof(char)); /* enough memory for the file */
    fread(inputBytes, inputLen, 1, F_INPUT); /* read in file */
    /* DEBUGGING */
    for (int i = 0; i < inputLen; i++) {
        printf("%d\n", inputBytes[i]);
    }


    /* PPM test implementation */
    const int WIDTH = 800, HEIGHT = 800;
    int x, y;
    FILE *F_OUTPUT = fopen(outputFile, "wb");
    fprintf(F_OUTPUT, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
    for (x = 0; x < WIDTH; x++) {
        for (y = 0; y < HEIGHT; y++) {
            static unsigned char colour[3];
            colour[0] = 255;
            colour[1] = 100;
            colour[2] = 100;
            fwrite(colour, 1, 3, F_OUTPUT);
        }
    }


    fclose(F_INPUT); fclose(F_OUTPUT);
    printf("Done\n");
    return 0;
}


/* return if incorrect usage */
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
