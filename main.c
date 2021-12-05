#include <stdio.h>
#include <stdlib.h>
/* for command line argument parsing and a couple other things*/
#include <unistd.h>

int badInput();

int main(int argc, char **argv) {

    /* command line argument parsing */
    int argIndex = 0;
    char *inputFile = NULL;
    char *outputFile = NULL;

    /* /\* begin implementing options - disabled for now *\/ */
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

    inputFile = argv[1];  /* first argument is input file */
    outputFile = argv[2]; /* second argument is output file */

    /* fail with wrong usage message if mandatory options missing */
    if (inputFile == NULL || outputFile == NULL) {
        return badInput("a");
    }

    FILE *file = fopen(inputFile, "r");
    /* check that input file exists and exit if not*/
    if (file == NULL) {
        return badInput("n");
    }
    /* read bytes to check if JPEG - error out if not */
    unsigned char bytes[3];
    fread(bytes, 3, 1, file);
    if (bytes[0] != 255 || bytes[1] != 216 || bytes[2] != 255) {
        return badInput("j");
    }

    printf("Done\n");

    return 0;
}


/* return if incorrect usage */
int badInput(char *errorType) {
    switch(*errorType) {
        case 'a':
            printf("Not enough arguments \n");
        break; case 'j' :
            printf("Input corrupted or not JPG \n");
        break; case 'n' :
            printf("Input file nonexistent \n");
        break; default:
            printf("Incorrect usage \n");
    }
    return 1;
}
