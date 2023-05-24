#include "MiniFB.h"
#include "IMG.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_SIZE (WINDOW_WIDTH * WINDOW_HEIGHT)

#define WHITE 0xFFFFFFFF
#define BLACK 0xFF000000
#define RED 0xFFFF0000
#define GREEN 0xFF00FF00
#define BLUE 0xFF0000FF

enum class PivotType {
    TopLeft,
    Center
};

class Window
{
private:
    /* data */
public:
    struct mfb_window *window;
    uint32_t* buffer;
    int state;
    Window(/* args */);
    void SetState();
    int GetState();
    void DrawPXL(int x, int y, uint32_t color);
    void DrawHorizontalLine(int length, int x, int y, uint32_t color);
    void DrawVerticalLine(int length, int x, int y, uint32_t color);
    void DrawRectShape(int width, int height, int x, int y, uint32_t color);
    void DrawFullRect(int width, int height, int x, int y, uint32_t color);
    void DrawIMG(IMG image, int posX, int posY);
    void DrawWholeWindow(uint32_t color);
    void DrawRotatedIMG(IMG image, int posX, int posY, float rotationAngle, PivotType pivotType);
    void DrawZoomedIMG(IMG image, int posX, int posY, float zoomFactor);
    void DrawZoomedAndRotatedIMG(IMG image, int posX, int posY, float zoomFactor, float rotationAngle);
    IMG ZoomedIMG(IMG image, float zoomFactor);
};

Window::Window(/* args */)
{
    window = mfb_open_ex("Lé PXL c rigolo", WINDOW_WIDTH, WINDOW_HEIGHT, WF_RESIZABLE);
    buffer = (uint32_t *) malloc(WINDOW_SIZE * 4);
}
void Window::SetState(){
    state = mfb_update_ex(window, buffer, WINDOW_WIDTH, WINDOW_HEIGHT);
}
int Window::GetState(){
    return state;
}
void Window::DrawPXL(int x, int y, uint32_t color)
{
    // while (x >= WINDOW_WIDTH)
    //     x -= WINDOW_WIDTH;
    // while (y >= WINDOW_HEIGHT)
    //     y -= WINDOW_HEIGHT;
    // while (x < 0)
    //     x += WINDOW_WIDTH;
    // while (y < 0)
    //     y += WINDOW_HEIGHT;

    if (x <= WINDOW_WIDTH && y <= WINDOW_HEIGHT && x >= 0 && y >= 0)
        buffer[y * WINDOW_WIDTH + x] = color;
}
void Window::DrawHorizontalLine(int length, int x, int y, uint32_t color)
{
    int end = x + length;
    for (; x < end; x++)
    {
        DrawPXL(x, y, color);
    }
}
void Window::DrawVerticalLine(int length, int x, int y, uint32_t color)
{
    int end = y + length;
    for (; y < end; y++)
    {
        DrawPXL(x, y, color);
    }
}
void Window::DrawRectShape(int width, int height, int x, int y, uint32_t color)
{
    DrawHorizontalLine(width,x,y,color);
    DrawHorizontalLine(width,x,y + height,color);
    DrawVerticalLine(height,x,y, color);
    DrawVerticalLine(height,x+ width,y, color);
}
void Window::DrawFullRect(int width, int height, int x, int y, uint32_t color)
{
    int end = height + y;
    for (; y < end; y++)
    {
        DrawHorizontalLine(width,x,y,color);
    }
}
void Window::DrawIMG(IMG image, int posX, int posY) {
    for (int y = 0; y < image.height; y++) {
        for (int x = 0; x < image.width; x++) {
            int idx = (y * image.width + x) * 4;
            uint32_t pixelColor = MFB_ARGB( image.img[idx+3],image.img[idx],
                                            image.img[idx+1], image.img[idx+2] );
            DrawPXL(x + posX, y + posY, pixelColor);
        }
    }
}
void Window::DrawWholeWindow(uint32_t color)
{
    memset(buffer, color, WINDOW_SIZE*4);
}
void Window::DrawRotatedIMG(IMG image, int posX, int posY, float rotationAngle, PivotType pivotType) {
    int idx = 0;
    
    float cosAngle = cos(rotationAngle);
    float sinAngle = sin(rotationAngle);

    int pivotX, pivotY;

    // Determine pivot coordinates based on pivotType
    switch (pivotType) {
        case PivotType::TopLeft:
            pivotX = 0;
            pivotY = 0;
            break;
        case PivotType::Center:
            pivotX = image.width / 2;
            pivotY = image.height / 2;
            break;
    }

    for (int y = 0; y < image.height; y++) {
        for (int x = 0; x < image.width; x++) {
            int relativeX = x - pivotX;
            int relativeY = y - pivotY;

            int rotatedX = static_cast<int>(cosAngle * relativeX - sinAngle * relativeY) + pivotX;
            int rotatedY = static_cast<int>(sinAngle * relativeX + cosAngle * relativeY) + pivotY;

            int targetX = rotatedX + posX;
            int targetY = rotatedY + posY;

            if (targetX >= 0 && targetX < WINDOW_WIDTH && targetY >= 0 && targetY < WINDOW_HEIGHT) {
                uint32_t pixelColor = (image.img[idx + 3] << 24 | image.img[idx] << 16 | image.img[idx + 1] << 8 | image.img[idx + 2]);
                DrawPXL(targetX, targetY, pixelColor);
            }
            idx += 4;
        }
    }
}
void Window::DrawZoomedIMG(IMG image, int posX, int posY, float zoomFactor) {
    int zoomedWidth = static_cast<int>(image.width * zoomFactor);
    int zoomedHeight = static_cast<int>(image.height * zoomFactor);
    
    for (int y = 0; y < zoomedHeight; y++) {
        for (int x = 0; x < zoomedWidth; x++) {
            int originalX = static_cast<int>(x / zoomFactor);
            int originalY = static_cast<int>(y / zoomFactor);

            if (originalX >= 0 && originalX < image.width && originalY >= 0 && originalY < image.height) {
                int idx = (originalY * image.width + originalX) * 4;
                uint32_t pixelColor = (image.img[idx + 3] << 24 | image.img[idx] << 16 | image.img[idx + 1] << 8 | image.img[idx + 2]);
                DrawPXL(x + posX, y + posY, pixelColor);
            }
        }
    }
}
IMG Window::ZoomedIMG(IMG image, float zoomFactor) {
    int zoomedWidth = static_cast<int>(image.width * zoomFactor);
    int zoomedHeight = static_cast<int>(image.height * zoomFactor);

    IMG zoomedImage;
    zoomedImage.width = zoomedWidth;
    zoomedImage.height = zoomedHeight;
    zoomedImage.channels = image.channels;
    zoomedImage.img = new unsigned char[zoomedWidth * zoomedHeight * zoomedImage.channels];

    for (int y = 0; y < zoomedHeight; y++) {
        for (int x = 0; x < zoomedWidth; x++) {
            int originalX = static_cast<int>(x / zoomFactor);
            int originalY = static_cast<int>(y / zoomFactor);

            if (originalX >= 0 && originalX < image.width && originalY >= 0 && originalY < image.height) {
                int idx = (originalY * image.width + originalX) * image.channels;
                int zoomedIdx = (y * zoomedWidth + x) * zoomedImage.channels;

                for (int c = 0; c < image.channels; c++) {
                    zoomedImage.img[zoomedIdx + c] = image.img[idx + c];
                }
            }
        }
    }

    return zoomedImage;
}
 