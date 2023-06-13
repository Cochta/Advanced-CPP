#include "IMG.h"
#include "Window.h"

class Entity
{
private:
    /* data */
public:
    int X, Y;
    float Scale = 1;
    int Speed = 1;
    float Rotation = 1;
    IMG img;
    Entity();
    Entity(int x, int y, IMG image, float scale, int speed, float rotation);
    void Draw(Window window);
};
Entity::Entity()
{

}
Entity::Entity(int x, int y, IMG image, float scale, int speed, float rotation)
{
    X = x;
    Y = y;
    img = image;
    Scale = scale;
    Speed = speed;
    Rotation = rotation;
}

void Entity::Draw(Window window)
{
    Y += Speed;
    window.DrawRotatedIMG(window.ZoomedIMG(img,Scale),X,Y,Rotation, PivotType::Center);
}
