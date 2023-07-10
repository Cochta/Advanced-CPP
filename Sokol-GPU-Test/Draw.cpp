#include "Draw.hpp"

float vertexBuffer[1000];
int vertexBufferUsed = 0;

uint32_t indexBuffer[1000];
int indexBufferUsed = 0;

const int NB_TILES = 4;
const float UV_TILE_SIZE = 1.0f / sqrt(NB_TILES);

Vector2F GlobalTranslation(0, 0);

float GlobalScaleRatio = 1;

float GlobalRotationAngle = 0;

void ClearDrawBuffers()
{
    vertexBufferUsed = 0;
    indexBufferUsed = 0;
}
void Translate(Vector2F translation)
{
    GlobalTranslation = translation;
}
void Rotate(float angle)
{
    GlobalRotationAngle = angle;
}
void Scale(float ratio)
{
    GlobalScaleRatio = ratio;
}
void AppendVertex(Vertex v)
{
    int winWidth = sapp_width();
    int winHeight = sapp_height();


    // Calculate the center of the window
    float centerX = winWidth / 2.0f;
    float centerY = winHeight / 2.0f;

    // Apply translation to make pivot the center of the window
    v.x -= centerX;
    v.y -= centerY;

    // Apply rotation
    float cosAngle = std::cos(GlobalRotationAngle);
    float sinAngle = std::sin(GlobalRotationAngle);
    float rotatedX = v.x * cosAngle - v.y * sinAngle;
    float rotatedY = v.x * sinAngle + v.y * cosAngle;

    // Apply scaling
    float scaledX = rotatedX * GlobalScaleRatio;
    float scaledY = rotatedY * GlobalScaleRatio;

    // Apply translation back
    v.x = scaledX + centerX + GlobalTranslation.X;
    v.y = scaledY + centerY + GlobalTranslation.Y;


    vertexBuffer[vertexBufferUsed++] = (v.x / winWidth) * 2 - 1;
    vertexBuffer[vertexBufferUsed++] = (v.y / winWidth) * 2 - 1;
    vertexBuffer[vertexBufferUsed++] = v.z;
    vertexBuffer[vertexBufferUsed++] = v.color.r;
    vertexBuffer[vertexBufferUsed++] = v.color.g;
    vertexBuffer[vertexBufferUsed++] = v.color.b;
    vertexBuffer[vertexBufferUsed++] = v.color.a;
    vertexBuffer[vertexBufferUsed++] = v.u;
    vertexBuffer[vertexBufferUsed++] = v.v;
}
void DrawQuad(Vector2F v1, Vector2F v2, Vector2F v3, Vector2F v4, Color c)
{
    int startIndex = vertexBufferUsed / (3 + 4 + 2);

    float z = 0.5f;

    AppendVertex(Vertex(v1.X, v1.Y, z, c, 0, 1));
    AppendVertex(Vertex(v2.X, v2.Y, z, c, 1, 1));
    AppendVertex(Vertex(v3.X, v3.Y, z, c, 1, 0));
    AppendVertex(Vertex(v4.X, v4.Y, z, c, 0, 0));

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
    int startIndex = vertexBufferUsed / (3 + 4 + 2);

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
    float dx = endX - startX;
    float dy = endY - startY;
    float length = std::sqrt(dx * dx + dy * dy);
    float angle = std::atan2(dy, dx);

    float halfThickness = thickness / 2.0f;
    float offsetX = halfThickness * std::sin(angle);
    float offsetY = halfThickness * -std::cos(angle);

    Vector2F v1 = Vector2F(startX + offsetX, startY + offsetY);
    Vector2F v2 = Vector2F(endX + offsetX, endY + offsetY);
    Vector2F v3 = Vector2F(endX - offsetX, endY - offsetY);
    Vector2F v4 = Vector2F(startX - offsetX, startY - offsetY);

    DrawQuad(v1, v2, v3, v4, c);
}
void DrawIMG(float x, float y, float width, float height, int index, Color c)
{
    int startIndex = vertexBufferUsed / (3 + 4 + 2);

    float z = 0.5f;

    Vector2F uv = GetTileUV(index);

    AppendVertex(Vertex(x, y, z, c, uv.X, uv.Y + UV_TILE_SIZE));
    AppendVertex(Vertex(x + width, y, z, c, uv.X + UV_TILE_SIZE, uv.Y + UV_TILE_SIZE));
    AppendVertex(Vertex(x + width, y + height, z, c, uv.X + UV_TILE_SIZE, uv.Y));
    AppendVertex(Vertex(x, y + height, z, c, uv.X, uv.Y));

    indexBuffer[indexBufferUsed++] = startIndex;
    indexBuffer[indexBufferUsed++] = startIndex + 1;
    indexBuffer[indexBufferUsed++] = startIndex + 2;

    indexBuffer[indexBufferUsed++] = startIndex;
    indexBuffer[indexBufferUsed++] = startIndex + 2;
    indexBuffer[indexBufferUsed++] = startIndex + 3;
}
Vector2F GetTileUV(int index)
{
    int tileRow = NB_TILES / 2;
    int idx = 0;
    for (int i = 0; i < tileRow; i++)
    {
        for (int j = 0; j < tileRow; j++)
        {
            idx++;
            if (idx == index)
            {
                return Vector2F(1.0f / (float)(tileRow - i), 1.0f / (float)(tileRow - j));
            }
        }
    }
    return Vector2F(0, 0);
}