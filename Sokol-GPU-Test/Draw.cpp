#include "Draw.hpp"

float vertexBuffer[1000];
int vertexBufferUsed = 0;

uint32_t indexBuffer[1000];
int indexBufferUsed = 0;

void ClearDrawBuffers()
{
    vertexBufferUsed = 0;
    indexBufferUsed = 0;
}
void AppendVertex(Vertex v)
{
    int winWidth = sapp_width();
    int winHeight = sapp_height();

    vertexBuffer[vertexBufferUsed++] = (v.x / winWidth) * 2 - 1;
    vertexBuffer[vertexBufferUsed++] = (v.y / winWidth) * 2 - 1;
    vertexBuffer[vertexBufferUsed++] = v.z;
    vertexBuffer[vertexBufferUsed++] = v.color.r;
    vertexBuffer[vertexBufferUsed++] = v.color.g;
    vertexBuffer[vertexBufferUsed++] = v.color.b;
    vertexBuffer[vertexBufferUsed++] = v.color.a;
}
void DrawQuad(Vector2F v1, Vector2F v2, Vector2F v3, Vector2F v4, Color c)
{
    int startIndex = vertexBufferUsed / (3 + 4);

    float z = 0.5f;

    AppendVertex(Vertex(v1.X, v1.Y, z, c));
    AppendVertex(Vertex(v2.X, v2.Y, z, c));
    AppendVertex(Vertex(v3.X, v3.Y, z, c));
    AppendVertex(Vertex(v4.X, v4.Y, z, c));

    indexBuffer[indexBufferUsed++] = startIndex;
    indexBuffer[indexBufferUsed++] = startIndex + 1;
    indexBuffer[indexBufferUsed++] = startIndex + 2;

    indexBuffer[indexBufferUsed++] = startIndex;
    indexBuffer[indexBufferUsed++] = startIndex + 2;
    indexBuffer[indexBufferUsed++] = startIndex + 3;
}
void DrawRect(float x, float y, float w, float h, Color c, Pivot p)
{
    if (p == Center)
    {
        x -= w / 2;
        y -= h / 2;
    }
    DrawQuad(Vector2F(x, y), Vector2F(x + w, y), Vector2F(x + w, y + h), Vector2F(x, y + h), c);
}
void DrawCircle(float x, float y, float r, Color c, int sides, bool pacman)
{
    const int numSegments = sides; // Adjust the number of segments for smoother circles

    float z = 0.5f;
    int startIndex = vertexBufferUsed / (3 + 4);

    // Center vertex
    AppendVertex(Vertex(x, y, z, c));

    // Calculate the angle between each segment
    float angleStep = (pacman ? 1.5f : 2.0f) * 3.14159f / numSegments;
    float bAngle = sides % 2 == 0 ? 0.0f : 3.14159f / (numSegments * 2.0f); // Make polygons symmetric on the screen
    float currentAngle = pacman ? 0.7f : bAngle;

    for (int i = 0; i <= numSegments; i++)
    {
        // Calculate the position of the current vertex
        float vertexX = x + r * cos(currentAngle);
        float vertexY = y + r * sin(currentAngle);

        // Append the vertex to the buffer
        AppendVertex(Vertex(vertexX, vertexY, z, c));

        // Add the indices to form triangles using the center vertex
        if (i > 0)
        {
            indexBuffer[indexBufferUsed++] = startIndex;
            indexBuffer[indexBufferUsed++] = startIndex + i;
            indexBuffer[indexBufferUsed++] = startIndex + i + 1;
        }

        // Update the current angle
        currentAngle += angleStep;
    }
}
void DrawPolygon(float x, float y, float size, Color c, int sides)
{
    DrawCircle(x, y, size / 2, c, sides);
}
void DrawLine(float startX, float startY, float endX, float endY, float thickness, Color c)
{
    float z = 0.5f;
    int startIndex = vertexBufferUsed / (3 + 4);

    float dx = endX - startX;
    float dy = endY - startY;
    float length = std::sqrt(dx * dx + dy * dy);
    float angle = std::atan2(dy, dx);

    float halfThickness = thickness / 2.0f;
    float offsetX = halfThickness * std::sin(angle);
    float offsetY = halfThickness * -std::cos(angle);

    float x1 = startX + offsetX;
    float y1 = startY + offsetY;
    float x2 = endX + offsetX;
    float y2 = endY + offsetY;
    float x3 = endX - offsetX;
    float y3 = endY - offsetY;
    float x4 = startX - offsetX;
    float y4 = startY - offsetY;

    DrawQuad(Vector2F(x1, y1), Vector2F(x2, y2), Vector2F(x3, y3), Vector2F(x4, y4), c);
}