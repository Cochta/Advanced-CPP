#include "Window.h"
#include "IMG.h"

class Bullet
{
private:
    /* data */
public:
    int X = 0, Y = 0;
    IMG img;
    float speed = 5;
    float angle = 0;
    Bullet(/* args */);
    Bullet(int x, int y, IMG image);
    void Draw(Window window);
};

Bullet::Bullet(/* args */)
{
}
Bullet::Bullet(int x, int y, IMG image)
{
    X = x;
    Y = y;
    img = image;
}
void Bullet::Draw(Window window){
    Y -= speed;
    angle += 0.1;
    window.DrawRotatedIMG(img,X,Y,angle, PivotType::Center);
}
