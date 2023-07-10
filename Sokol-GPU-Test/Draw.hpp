#pragma once
#include "Engine.hpp"
#include "IMG.h"
enum Pivot
{
    Center,
    TopLeft,
};

void ClearDrawBuffers();
void Translate(Vector2F translation);
void Rotate(float angle);
void Scale(float ratio);
void DrawRect(float x, float y, float w, float h, Color c, Pivot p = Center);
void DrawCircle(float x, float y, float r, Color c, int sides = 50, bool pacman = false);
void DrawPolygon(float x, float y, float size, Color c, int sides);
void DrawLine(float startX, float startY, float endX, float endY, float thickness, Color c);
void DrawIMG(float x, float y, float width, float height, int index, Color c);
Vector2F GetTileUV(int index);