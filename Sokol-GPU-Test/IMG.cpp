#include "IMG.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

IMG::IMG(const char* path)
{
    int dummy;
    img = stbi_load(path, &width, &height, &dummy, 0);
    channels = 4;
}
IMG::IMG(){
    
}
