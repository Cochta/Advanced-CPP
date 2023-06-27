#pragma once
#include "Engine.hpp"

enum Pivot
{
    Center,
    TopLeft,
};

void ClearDrawBuffers();
void DrawRect(float x, float y, float w, float h, Color c, Pivot p = Center);
void DrawCircle(float x, float y, float r, Color c, int sides = 50, bool pacman = false);
void DrawPolygon(float x, float y, float size, Color c, int sides);
void DrawLine(float startX, float startY, float endX, float endY, float thickness, Color c);