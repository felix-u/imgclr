#include <stdio.h>
#include <stdlib.h>
#include <ImageMagick-7/MagickWand/MagickWand.h>

int main() {

   MagickWand *mv = NULL;
   mv = NewMagickWand();
   MagickReadImage(mv,"testimage.jpg");
   MagickWriteImage(mv,"newtestimage.jpg");

   printf("Done\n");

   return 0;
}
