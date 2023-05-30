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

    Player player;
    int fireRate = player.fireRate+1;
    int enemySpawnRate = 45;
    int enemySpawn = enemySpawnRate;
    IMG font("ressources/font_map.png");
    IMG rat("ressources/Rat.png");
    IMG heart("ressources/Heart.png");
    Enemy enemies[100];
    float enemyScale = 2.5f;
    int enemyCounter = 0;
    int score = 0;
    int HP = 5;
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
            if ((IsKeyDown(KB_KEY_W) || IsKeyDown(KB_KEY_UP)) && player.y > 0)
                player.y -= 5 ;
            if ((IsKeyDown(KB_KEY_S)|| IsKeyDown(KB_KEY_DOWN))&& player.y + player.img.height*player.Scale < WINDOW_HEIGHT)
                player.y += 5 ;
            if (WasKeyJustPressed(KB_KEY_SPACE) && fireRate >= player.fireRate)
            {
                player.SpawnBullet(player.x,player.y);
                fireRate = 0;
            }
            enemySpawn++;
            if (enemySpawn >= enemySpawnRate && enemyCounter < 100)
            {
                enemies[enemyCounter] = Enemy(rand() % WINDOW_WIDTH-rat.width * enemyScale/2,-100,rat, enemyScale);
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
                        score += 100;
                        
                        break;
                    }
                }

                if (enemies[i].Y > WINDOW_HEIGHT)
                {
                    enemies[i] = enemies[enemyCounter-1];
                    enemyCounter--;
                    HP -= 1;
                    if (HP <= 0)
                    {
                        game = false;
                    }
                }
                enemies[i].Draw(myWindow);
            }
            myWindow.DrawFullRect(WINDOW_WIDTH, 50, 0,0, GREEN);
            myWindow.DrawZoomedIMG(heart, 0,0, 1.6f);
            myWindow.DrawText("= " + std::to_string(HP) , 32*2,20);
            myWindow.DrawText("score = " + std::to_string(score) , WINDOW_WIDTH / 2,20);
            player.Draw(myWindow);
        }
        else // GAME OVER
        {
            myWindow.DrawFullRect(WINDOW_WIDTH, 150, 0,WINDOW_HEIGHT/2-75, GREEN);
            myWindow.DrawText("! GAME OVER !", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 15);
            myWindow.DrawText("score = " + std::to_string(score), WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
            myWindow.DrawText("Press ENTER to play again", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2+15);
            if (IsKeyDown(KB_KEY_ENTER))
            {
                game = true;
                score = 0;
                HP = 5;
            }
        }

        myWindow.SetState();

        if (myWindow.GetState() < 0) {
            myWindow.window = NULL;
            break;
        }
    } while(mfb_wait_sync(myWindow.window));
}