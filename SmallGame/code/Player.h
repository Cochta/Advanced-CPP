#include "Window.h"
#include "Bullet.h"
class Player
{
private:
    /* data */
public:
    int fireRate = 20;
    float Scale = 1;
    IMG img;
    IMG beer;
    int x, y;
    Bullet bullets[20];
    int bulletCount = 0;
    Player(/* args */);
    void Draw(Window window);
    void SpawnBullet(int x, int y);
    void DestroyBullet(int index);
};

Player::Player(/* args */)
{
    img = IMG("ressources/Bum.png");
    beer = IMG("ressources/Beer.png");
    Scale = 3;
    x = WINDOW_WIDTH / 2, y = WINDOW_HEIGHT - img.height * Scale;
}
void Player::Draw(Window window)
{
    for (int i = 0; i < bulletCount; i++)
    {
        if (bullets[i].Y < - 100)
        {
            DestroyBullet(i);
        }
        bullets[i].Draw(window);
    }
    window.DrawZoomedIMG(img,x,y, Scale);
}
void Player::SpawnBullet(int x, int y){

    if (bulletCount == 20)
        return;

    bullets[bulletCount] = Bullet(x+ img.width/2 * Scale - beer.width/2, y, beer);
    bulletCount++;
}
void Player::DestroyBullet(int index){
    bullets[index] = bullets[bulletCount-1];
    bulletCount--;
}