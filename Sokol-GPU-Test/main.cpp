#include "Engine.hpp"
#include "Draw.hpp"

int frameCount = 0;

void GameFrame()
{
    DrawRect(50, 50, 100, 100, CYAN);
    DrawCircle(400, 400, 100, YELLOW);
    DrawPolygon(100, 400, 200, MAGENTA, 9);
    DrawPolygon(750, 750, 100, BLUE, 11);
    DrawLine(1,0,1,800,1,BLUE);
    DrawLine(0,0,800,0,1,BLUE);
    DrawLine(800,799,0,799,1,BLUE);
    DrawLine(800,800,800,0,1,BLUE);
    frameCount++;
}
