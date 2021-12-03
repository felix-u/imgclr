#include <stdio.h>
#include <stdlib.h>
/* for imagemagick */
#include <ImageMagick-7/MagickWand/MagickWand.h>
/* for command line argument parsing and a couple other things*/
#include <unistd.h>

int main(int argc, char **argv) {

  /* command line argument parsing */
  int argIndex = 0;
  char *inputFile = NULL;
  char *outputFile = NULL;

  /* return if incorrect usage */
  int badInput() {
    printf("Incorrect usage \n");
    return 1;
  }

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
    return badInput();
  }

  /* check if input is valid */
  if (access(inputFile, F_OK) != 0) {
    printf("Nonexistent input file \n");
    return 1;
  }

  /* initialise MagickWand */
  MagickWand *mw = NULL;
  mw = NewMagickWand();

  MagickReadImage(mw, inputFile);
  MagickWriteImage(mw, outputFile);

  printf("Done\n");

  mw = DestroyMagickWand(mw);

  return 0;
}
