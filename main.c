#include <stdio.h>
#include <stdlib.h>
/* for imagemagick */
#include <ImageMagick-7/MagickWand/MagickWand.h>
/* for command line argument parsing */
#include <getopt.h>

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

  while ((argIndex = getopt(argc, argv, "i:o")) != -1) {
    switch (argIndex) {
    case 'i':
      inputFile = optarg + 1;
      printf("inputFile is %s\n", inputFile);
      break;
    case 'o':
      outputFile = optarg + 1;
      printf("outputFile is %s\n", outputFile);
      break;
    default:
      return badInput();
    }
  }

  /* fail with wrong usage message if mandatory options missing */
  if (inputFile == NULL || outputFile == NULL) {
    return badInput();
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
