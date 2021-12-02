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

   while ((argIndex = getopt(argc, argv, "i:o")) != -1 ) {
      switch (argIndex) {
         case 'i':
            inputFile = optarg;
            printf("Input argument detected \n");
         break; case 'o':
            outputFile = optarg;
            printf("Output file detected \n");
         break; default:
            printf("Option error \n");
            return 1;
      }
   }

   /* MagickWand *mw = NULL; */
   /* mw = NewMagickWand(); */

   /* MagickReadImage(mw,"testimage.jpg"); */
   /* MagickWriteImage(mw,"newtestimage.jpg"); */

   /* printf("Done\n"); */

   /* mw = DestroyMagickWand(mw); */
   return 0;
}
