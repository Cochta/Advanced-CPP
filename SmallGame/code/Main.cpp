#include <iostream>
#include "MiniFB.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_SIZE (WINDOW_WIDTH * WINDOW_HEIGHT)

#define WHITE 0xFFFFFFFF
#define BLACK 0xFF000000
#define RED 0xFFFF0000
#define GREEN 0xFF00FF00
#define BLUE 0xFF0000FF

uint32_t* buffer;

void DrawPXL(int x, int y, uint32_t color)
{
    if (x >= WINDOW_WIDTH)
        x = WINDOW_WIDTH-1;
    if (y >= WINDOW_HEIGHT)
        y = WINDOW_HEIGHT-1;
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;

    buffer[y * WINDOW_WIDTH + x] = color;
}

void DrawHorizontalLine(int length, int x, int y, uint32_t color)
{
    int end = x + length;
    for (; x < end; x++)
    {
        DrawPXL(x, y, color);
    }
}

void DrawVerticalLine(int length, int x, int y, uint32_t color)
{
    int end = y + length;
    for (; y < end; y++)
    {
        DrawPXL(x, y, color);
    }
}

void DrawRectShape(int width, int height, int x, int y, uint32_t color)
{
    DrawHorizontalLine(width,x,y,color);
    DrawHorizontalLine(width,x,y + height,color);
    DrawVerticalLine(height,x,y, color);
    DrawVerticalLine(height,x+ width,y, color);
}

void DrawFullRect(int width, int height, int x, int y, uint32_t color)
{
    int end = height + y;
    for (; y < end; y++)
    {
        DrawHorizontalLine(width,x,y,color);
    }
}

void DrawWholeWindow(uint32_t color)
{
    for (int i = 0; i < WINDOW_SIZE; i++)
    {
        buffer[i] = color;
    }
}

int main(){
    struct mfb_window *window = mfb_open_ex("my display", WINDOW_WIDTH, WINDOW_HEIGHT, WF_RESIZABLE);
if (!window)
    return 0;

buffer = (uint32_t *) malloc(WINDOW_SIZE * 4);

int x = 0;
do {
    int state;

    DrawWholeWindow(BLACK);

    //DrawHorizontalLine(200,20,20,GREEN);
    //DrawVerticalLine(200,20,20,GREEN);

    DrawRectShape(100,200,20+x,20,RED);

    DrawFullRect(100,200,400+x,100, GREEN);

    DrawRectShape(WINDOW_WIDTH,WINDOW_HEIGHT,0,0,BLACK);

    // TODO: add some fancy rendering to the buffer of size 800 * 600

    state = mfb_update_ex(window, buffer, WINDOW_WIDTH, WINDOW_HEIGHT);

    if (state < 0) {
        window = NULL;
        break;
    }
    x++;
} while(mfb_wait_sync(window));
}