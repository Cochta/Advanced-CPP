#pragma once

#include "Vector2.h"

struct Color
{
    float r, g, b, a;
    Color(float rVal, float gVal, float bVal, float aVal) : r(rVal), g(gVal), b(bVal), a(aVal) {}
};
const Color BLACK(0, 0, 0, 1);
const Color WHITE(1, 1, 1, 1);
const Color RED(1, 0, 0, 1);
const Color GREEN(0, 1, 0, 1);
const Color BLUE(0, 0, 1, 1);
const Color CYAN(0, 1, 1, 1);
const Color YELLOW(1, 1, 0, 1);
const Color MAGENTA(1, 0, 1, 1);

struct Vertex
{
    float x, y, z;
    Color color;

    Vertex(float xVal, float yVal, float zVal, Color cVal) : x(xVal), y(yVal), z(zVal), color(cVal) {}
};
