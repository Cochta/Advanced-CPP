#include "IMG.h"
#include "Window.h"

class Enemy
{
private:
    /* data */
public:
    int X, Y;
    float Scale = 1;
    int speed = 3;
    IMG img;
    Enemy();
    Enemy(int x, int y, IMG image, float scale);
    void Draw(Window window);
};
Enemy::Enemy()
{

}
Enemy::Enemy(int x, int y, IMG image, float scale)
{
    X = x;
    Y = y;
    img = image;
    Scale = scale;
}

void Enemy::Draw(Window window)
{
    Y += speed;
    window.DrawZoomedIMG(img,X,Y, Scale);
}
