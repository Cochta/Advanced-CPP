#pragma once
#include "MiniFB.h"
#include "IMG.h"
#include "iostream"
#define SOKOL_IMPL
#include "sokol_audio.h"
#define SOKOL_LOG_IMPL
#include "sokol_log.h"

#include "microui.h"

#define WINDOW_FAC 4

#define FRAME_WIDTH 200

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define WINDOW_SIZE (WINDOW_WIDTH * WINDOW_HEIGHT)

#define WHITE 0xFFFFFFFF
#define BLACK 0xFF000000
#define RED 0xFFFF0000
#define GREEN 0xFF00FF00
#define BLUE 0xFF0000FF

enum class PivotType
{
    TopLeft,
    Center
};

class Window
{
private:
    /* data */
public:
    IMG Font;
    struct mfb_window *window;
    uint32_t *buffer;
    int state;
    bool xScrolling = false, yScrolling = false;
    Window(/* args */);
    void SetState();
    int GetState();
    void SetScrolling(bool xScrolling, bool yScrolling);
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
    void DrawLetter(char letter, int posX, int posY);
    void DrawText(std::string text, int x, int y, PivotType pivotType);
    void resize_bitmap(uint32_t *dest, int dest_sx, int dest_sy, uint32_t *src, int src_sx, int src_sy);
};

Window::Window(/* args */)
{
    window = mfb_open_ex("Game", WINDOW_WIDTH, WINDOW_HEIGHT, WF_RESIZABLE);
    buffer = (uint32_t *)malloc(WINDOW_SIZE * 4);
    Font = ZoomedIMG(IMG("ressources/font_map.png"), 3);
}
void Window::SetState()
{
    state = mfb_update_ex(window, buffer, WINDOW_WIDTH, WINDOW_HEIGHT);
}
int Window::GetState()
{
    return state;
}
void Window::SetScrolling(bool x, bool y)
{
    xScrolling = x;
    yScrolling = y;
}
void Window::DrawPXL(int x, int y, uint32_t color)
{
    if (xScrolling)
    {
        while (x >= WINDOW_WIDTH)
            x -= WINDOW_WIDTH;
        while (x < 0)
            x += WINDOW_WIDTH;
    }
    else
    {
        if (x >= WINDOW_WIDTH || x < 0)
            return;
    }
    if (yScrolling)
    {
        while (y >= WINDOW_HEIGHT)
            y -= WINDOW_HEIGHT;

        while (y < 0)
            y += WINDOW_HEIGHT;
    }
    else
    {
        if (y >= WINDOW_HEIGHT || y < 0)
            return;
    }

    int pixel_index = y * WINDOW_WIDTH + x;
    uint32_t dest_color = buffer[pixel_index];

    float src_alpha = ((uint8_t)(color >> 24)) / 256.f;
    uint8_t src_r = (uint8_t)(color >> 16);
    uint8_t src_g = (uint8_t)(color >> 8);
    uint8_t src_b = (uint8_t)(color);

    uint8_t dest_r = (uint8_t)(dest_color >> 16);
    uint8_t dest_g = (uint8_t)(dest_color >> 8);
    uint8_t dest_b = (uint8_t)(dest_color);

    uint8_t final_r = (uint8_t)((src_r * src_alpha + dest_r * (1.f - src_alpha)));
    uint8_t final_g = (uint8_t)((src_g * src_alpha + dest_g * (1.f - src_alpha)));
    uint8_t final_b = (uint8_t)((src_b * src_alpha + dest_b * (1.f - src_alpha)));

    buffer[pixel_index] = MFB_ARGB(255, final_r, final_g, final_b);
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
    DrawHorizontalLine(width, x, y, color);
    DrawHorizontalLine(width, x, y + height, color);
    DrawVerticalLine(height, x, y, color);
    DrawVerticalLine(height, x + width, y, color);
}
void Window::DrawFullRect(int width, int height, int x, int y, uint32_t color)
{
    int end = height + y;
    for (; y < end; y++)
    {
        DrawHorizontalLine(width, x, y, color);
    }
}
void Window::DrawIMG(IMG image, int posX, int posY)
{
    for (int y = 0; y < image.height; y++)
    {
        for (int x = 0; x < image.width; x++)
        {
            int idx = (y * image.width + x) * 4;
            uint32_t pixelColor = MFB_ARGB(image.img[idx + 3], image.img[idx],
                                           image.img[idx + 1], image.img[idx + 2]);
            DrawPXL(x + posX, y + posY, pixelColor);
        }
    }
}
void Window::DrawLetter(char letter, int posX, int posY)
{
    int startX = 0, startY = 0;

    int zoom = 3;
    if (letter >= '!')
    {
        startX = 0 + (letter - '!') * 7 * zoom;
        startY = 22 * zoom;
    }
    if (letter >= 'A')
    {
        startX = 0 + (letter - 'A') * 7 * zoom;
        startY = 0;
    }
    if (letter >= 'a')
    {
        startX = 0 + (letter - 'a') * 7 * zoom;
        startY = 12 * zoom;
    }
    if (letter == ' ')
    {
        startX = 0;
        startY = 50 * zoom;
    }
    for (int y = startY; y < startY + 9 * zoom; y++)
    {
        for (int x = startX; x < startX + 7 * zoom; x++)
        {
            int idx = (y * Font.width + x) * 4;
            uint32_t pixelColor = MFB_ARGB(Font.img[idx + 3], Font.img[idx],
                                           Font.img[idx + 1], Font.img[idx + 2]);
            DrawPXL(posX + x - startX, posY + y - startY, pixelColor);
        }
    }
}
void Window::DrawWholeWindow(uint32_t color)
{
    for (int i = 0; i < WINDOW_SIZE; i++)
    {

        buffer[i] = color;
    }
}
void Window::DrawRotatedIMG(IMG image, int posX, int posY, float rotationAngle, PivotType pivotType)
{
    int idx = 0;

    float cosAngle = cos(rotationAngle);
    float sinAngle = sin(rotationAngle);

    int pivotX, pivotY;

    // Determine pivot coordinates based on pivotType
    switch (pivotType)
    {
    case PivotType::TopLeft:
        pivotX = 0;
        pivotY = 0;
        break;
    case PivotType::Center:
        pivotX = image.width / 2;
        pivotY = image.height / 2;
        break;
    }

    for (int y = 0; y < image.height; y++)
    {
        for (int x = 0; x < image.width; x++)
        {
            int relativeX = x - pivotX;
            int relativeY = y - pivotY;

            float rotatedXFloat = cosAngle * relativeX - sinAngle * relativeY + pivotX + posX;
            float rotatedYFloat = sinAngle * relativeX + cosAngle * relativeY + pivotY + posY;

            int targetX = static_cast<int>(rotatedXFloat);
            int targetY = static_cast<int>(rotatedYFloat);

            if (targetX >= 0 && targetX < WINDOW_WIDTH && targetY >= 0 && targetY < WINDOW_HEIGHT)
            {
                uint8_t alpha = image.img[idx + 3];
                if (alpha != 0)
                {
                    uint32_t pixelColor = (alpha << 24 | image.img[idx] << 16 | image.img[idx + 1] << 8 | image.img[idx + 2]);
                    DrawPXL(targetX, targetY, pixelColor);
                }
            }
            idx += 4;
        }
    }
}
void Window::DrawZoomedIMG(IMG image, int posX, int posY, float zoomFactor)
{
    int zoomedWidth = static_cast<int>(image.width * zoomFactor);
    int zoomedHeight = static_cast<int>(image.height * zoomFactor);

    for (int y = 0; y < zoomedHeight; y++)
    {
        for (int x = 0; x < zoomedWidth; x++)
        {
            int originalX = static_cast<int>(x / zoomFactor);
            int originalY = static_cast<int>(y / zoomFactor);

            if (originalX >= 0 && originalX < image.width && originalY >= 0 && originalY < image.height)
            {
                int idx = (originalY * image.width + originalX) * 4;
                uint32_t pixelColor = (image.img[idx + 3] << 24 | image.img[idx] << 16 | image.img[idx + 1] << 8 | image.img[idx + 2]);
                DrawPXL(x + posX, y + posY, pixelColor);
            }
        }
    }
}
IMG Window::ZoomedIMG(IMG image, float zoomFactor)
{
    int zoomedWidth = static_cast<int>(image.width * zoomFactor);
    int zoomedHeight = static_cast<int>(image.height * zoomFactor);

    IMG zoomedImage;
    zoomedImage.width = zoomedWidth;
    zoomedImage.height = zoomedHeight;
    zoomedImage.channels = image.channels;
    zoomedImage.img = new unsigned char[zoomedWidth * zoomedHeight * zoomedImage.channels];

    for (int y = 0; y < zoomedHeight; y++)
    {
        for (int x = 0; x < zoomedWidth; x++)
        {
            int originalX = static_cast<int>(x / zoomFactor);
            int originalY = static_cast<int>(y / zoomFactor);

            if (originalX >= 0 && originalX < image.width && originalY >= 0 && originalY < image.height)
            {
                int idx = (originalY * image.width + originalX) * image.channels;
                int zoomedIdx = (y * zoomedWidth + x) * zoomedImage.channels;

                for (int c = 0; c < image.channels; c++)
                {
                    zoomedImage.img[zoomedIdx + c] = image.img[idx + c];
                }
            }
        }
    }

    return zoomedImage;
}
void Window::DrawText(std::string text, int x, int y, PivotType pivotType = PivotType::Center)
{
    int zoom = 3;
    int posX = 0;
    if (pivotType == PivotType::Center)
        posX = -text.size() / 2 * 7 * zoom;

    for (auto letter : text)
    {
        DrawLetter(letter, x + posX, y);
        posX += 7 * zoom;
    }
}
void Window::resize_bitmap(uint32_t *dest, int dest_sx, int dest_sy, uint32_t *src, int src_sx, int src_sy)
{
    for (int y = 0; y < dest_sy; y++)
    {
        for (int x = 0; x < dest_sx; x++)
        {
            int src_x = x * src_sx / dest_sx;
            int src_y = y * src_sy / dest_sy;
            dest[y * dest_sx + x] = src[src_y * src_sx + src_x];
        }
    }
}