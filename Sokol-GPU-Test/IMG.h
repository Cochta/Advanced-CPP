#pragma once

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

