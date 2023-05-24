#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class IMG
{
private:
    /* data */
public:
    unsigned char *img;
    int width = 0, height = 0, channels = 4;
    IMG();
    IMG(const char* path);
    
};

IMG::IMG(const char* path)
{
    img = stbi_load(path, &width, &height, &channels, 0);
}
IMG::IMG(){
    
}
