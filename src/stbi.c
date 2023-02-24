/* @Feature { stb_image can read PNM but stb_image_write cannot write it. PPM is
              quite simple, so maybe I should implement my own writer.
} */
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_PNM
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "../libs/stb_image-v2.27/stb_image.h"
#include "../libs/stb_image_write-v1.16/stb_image_write.h"

