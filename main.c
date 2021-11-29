#include <stdio.h>
#include <stdlib.h>
#include <ImageMagick-7/MagickWand/MagickWand.h>

int main() {
   // printf() displays the string inside quotation
   //
   printf("Hello, World!");

   MagickWand *mv = NULL;
   mv = NewMagickWand();
   MagickReadImage(mv,"testimage.jpg");
   MagickWriteImage(mv,"newtestimage.jpg");

   return 0;
}
