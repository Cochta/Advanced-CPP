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

struct sapp_event;

struct Vertex
{
    float x, y, z;
    Color color;
    float u, v;

    Vertex(float xVal, float yVal, float zVal, Color cVal, float uVal = 1, float vVal = 1) : x(xVal), y(yVal), z(zVal), color(cVal), u(uVal), v(vVal) {}
};
void OnInput(const sapp_event *event);

void Update();
bool IsKeyPressed(int key);
bool IsKeyReleased(int key);
bool IsKeyDown(int key);
bool IsMouseButtonPressed(int button);
bool IsMouseButtonReleased(int button);
bool IsMouseButtonDown(int button);

Vector2F GetMousePosition();
Vector2F GetPreviousMousePosition();
Vector2F ConvertInputPosition(Vector2F position);