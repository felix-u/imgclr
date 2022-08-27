#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
// @Missing stb_image can rean PNM but stb_image_write cannot write it. I
// should implement my own PPM writer. @Missing
#define STBI_ONLY_PNM
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

