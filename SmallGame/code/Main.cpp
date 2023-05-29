#include <iostream>
#include "stdio.h"
#include "string.h"
#include <sstream>
#include "Player.h"
#include "Enemy.h"

bool keys_state[KB_KEY_LAST+1];
bool keys_state_prev[KB_KEY_LAST+1];

void TickInput(){
    memcpy(keys_state_prev, keys_state, sizeof(keys_state));
}
bool IsKeyDown(mfb_key key){
    return keys_state[key];
}
bool WasKeyJustPressed(mfb_key key){
    return !keys_state_prev[key] && keys_state[key];
}
bool WasKeyJustReleased(mfb_key key){
    return keys_state_prev[key] && !keys_state[key];
}
void OnKeyboardEvent(struct mfb_window *window, mfb_key key, mfb_key_mod mod, bool isPressed)
{
    (void) window;
    (void) mod;
    keys_state[key] = isPressed;
}

int main(){
    bool menu = true;
    bool game = false;

    Window myWindow = Window();
    mfb_set_keyboard_callback(myWindow.window, OnKeyboardEvent);
#pragma region 
    // IMG beer("ressources/Beer.png");
    // IMG rat("ressources/Rat.png");
    // IMG heart("ressources/Heart.png");
    // IMG banane("ressources/Banane.png");

    // float rotationFactor = 0;
    // float zoomFactor = 3;

    // float maxZoom = 10;
    // float minZoom = 2;

    // bool zoom = true;
#pragma endregion

    Player player;
    int fireRate = player.fireRate+1;
    int enemySpawnRate = 30;
    int enemySpawn = enemySpawnRate;
    IMG font("ressources/font_map.png");
    IMG rat("ressources/Rat.png");
    Enemy enemies[100];
    float enemyScale = 2.5f;
    int enemyCounter = 0;
    do {
        if (IsKeyDown(KB_KEY_ESCAPE))
            break;
        
        myWindow.DrawWholeWindow(0xFF3A75C5);
        if (menu)
        {
            myWindow.DrawRectShape(300,50,WINDOW_WIDTH/2 -150,50, GREEN);
            myWindow.DrawText("SPACE BUM", WINDOW_WIDTH/2,60);
            myWindow.DrawText("!!!          VS          !!!", WINDOW_WIDTH/2,70);
            myWindow.DrawText("SPACE RAT INVASION", WINDOW_WIDTH/2,80);

            myWindow.DrawRectShape(300,50,WINDOW_WIDTH/2 -150,300, GREEN);
            myWindow.DrawText("Press ENTER to start", WINDOW_WIDTH/2,325);
            if (IsKeyDown(KB_KEY_ENTER))
            {
                menu = false;
                game = true;
            }
        }
        else if (game)
        {
            fireRate++;
            if ((IsKeyDown(KB_KEY_A) || IsKeyDown(KB_KEY_LEFT)) && player.x > 0)
                player.x -= 5 ;
            if ((IsKeyDown(KB_KEY_D)|| IsKeyDown(KB_KEY_RIGHT))&& player.x + player.img.width*player.Scale < WINDOW_WIDTH)
                player.x += 5 ;
            if (WasKeyJustPressed(KB_KEY_SPACE) && fireRate >= player.fireRate)
            {
                player.SpawnBullet(player.x,player.y);
                fireRate = 0;
            }
            enemySpawn++;
            if (enemySpawn >= enemySpawnRate && enemyCounter < 100)
            {
                enemies[enemyCounter] = Enemy(rand() % WINDOW_WIDTH-rat.width * enemyScale,-100,rat, enemyScale);
                enemySpawn = 0;
                enemyCounter++;
            }
            for (int i = 0; i < enemyCounter; i++)
            {
                for (int j = 0; j < player.bulletCount; j++)
                {
                    if (player.bullets[j].X < enemies[i].X + enemies[i].img.width * enemies[i].Scale &&
                        player.bullets[j].X + player.bullets[j].img.width > enemies[i].X &&
                        player.bullets[j].Y < enemies[i].Y + enemies[i].img.height * enemies[i].Scale &&
                        player.bullets[j].Y + player.bullets[j].img.height > enemies[i].Y)
                    {
                        player.DestroyBullet(j);

                        enemies[i] = enemies[enemyCounter-1];
                        enemyCounter--;
                        
                        break;
                    }
                }

                if (enemies[i].Y > WINDOW_HEIGHT)
                {
                    enemies[i] = enemies[enemyCounter-1];
                    enemyCounter--;
                }
                
                enemies[i].Draw(myWindow);
            }
            player.Draw(myWindow);
        }
#pragma region 
        // for (int i = -1; i < WINDOW_WIDTH / rat.width / zoomFactor + 1; i++)
        // {
        //     for (int j = -1; j < WINDOW_HEIGHT / rat.height / zoomFactor + 1; j++)
        //     {
        //         myWindow.DrawRotatedIMG(myWindow.ZoomedIMG(beer, zoomFactor),
        //         rat.width * i * zoomFactor, rat.height * j * zoomFactor, rotationFactor, PivotType::TopLeft);
        //     }
        // }
        
        // myWindow.DrawIMG(beer, 100, 100);
        // myWindow.DrawRotatedIMG(rat, 200, 200, 1.5f);
        // myWindow.DrawZoomedIMG(heart, 300, 300, 7.2f);

        // myWindow.DrawRotatedIMG(myWindow.ZoomedIMG(rat, 7.5f), 400,200, 1.5f+ x);
        // myWindow.DrawIMG(myWindow.ZoomedIMG(rat, 7.5f), 100, 100);

        // myWindow.DrawRectShape(400,200,0+x,20,RED);
        // myWindow.DrawFullRect(100,400,500,300+x, GREEN);
        // myWindow.DrawRectShape(WINDOW_WIDTH,WINDOW_HEIGHT,0,0,BLACK);
        
        // if (zoomFactor >= maxZoom)
        //     zoom = false;
        // else if (zoomFactor <= minZoom)
        //     zoom = true;
        
        // if (zoom)
        //     zoomFactor +=0.5f;
        // else
        //     zoomFactor -=0.5f;

        // rotationFactor -= 0.1f;
#pragma endregion

        myWindow.SetState();

        if (myWindow.GetState() < 0) {
            myWindow.window = NULL;
            break;
        }
    } while(mfb_wait_sync(myWindow.window));
}