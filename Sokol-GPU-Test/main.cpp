#include "Engine.hpp"
#include "Draw.hpp"

int frameCount = 0;
Vector2F translation(0, 0);
float angle = 0;
float ratio = 1;
void GameFrame()
{
    Update();
    if (IsKeyDown(87))
    {
        translation.Y += 10;
    }
    if (IsKeyDown(83))
    {
        translation.Y -= 10;
    }
    if (IsKeyDown(68))
    {
        translation.X += 10;
    }
    if (IsKeyDown(65))
    {
        translation.X -= 10;
    }
    if (IsKeyReleased(32))
    {
        printf("pipi");
        translation.X = 0;
        translation.Y = 0;
    }

    Translate(translation);
    Rotate(angle);
    Scale(ratio);
    DrawRect(300, 300, 50, 50, WHITE);
    DrawIMG(500, 500, 50, 50, 3, WHITE);
    DrawCircle(400, 400, 100, MAGENTA);
    // DrawPolygon(100, 400, 200, MAGENTA, 9);
    // DrawPolygon(750, 750, 100, BLUE, 11);
    // DrawLine(100, 50, 400, 600, 1, BLUE);
    frameCount++;
}
