#include <iostream>
#include "stdio.h"
#include "string.h"
#include <sstream>
#include "Player.h"
#include "Entity.h"
#define _USE_MATH_DEFINES
#include "math.h"
#include "GSound.h"
// #include <unordered_map>

#define ARR_LEN(arr) ((int) (sizeof(arr)/ sizeof(*arr)))
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

#define xstr(a) str(a)
#define str(a) #a
#define LOG(message) printf(message "(" __FILE__ ": " str(__LINE__) ")\n" )

bool keys_state[KB_KEY_LAST + 1];
bool keys_state_prev[KB_KEY_LAST + 1];

void TickInput()
{
    memcpy(keys_state_prev, keys_state, sizeof(keys_state));
}
bool IsKeyDown(mfb_key key)
{
    return keys_state[key];
}
bool WasKeyJustPressed(mfb_key key)
{
    return !keys_state_prev[key] && keys_state[key];
}
bool WasKeyJustReleased(mfb_key key)
{
    return keys_state_prev[key] && !keys_state[key];
}
void OnKeyboardEvent(struct mfb_window *window, mfb_key key, mfb_key_mod mod, bool isPressed)
{
    (void)window;
    (void)mod;
    keys_state[key] = isPressed;
}
// float N(const std::string& note, int octave) {
//     // Map of note names to their corresponding frequencies
//     std::unordered_map<std::string, float> noteFrequencies = {
//         {"SI#", 261.63f},
//         {"DO", 261.63f},
//         {"DO#", 277.18f},
//         {"REb", 277.18f},
//         {"RE", 293.66f},
//         {"RE#", 311.13f},
//         {"MIb", 311.13f},
//         {"MI", 329.63f},
//         {"FAb", 329.63f},
//         {"FA", 349.23f},
//         {"MI#", 349.23f},
//         {"FA#", 369.99f},
//         {"SOLb", 369.99f},
//         {"SOL", 392.00f},
//         {"SOL#", 415.30f},
//         {"LAb", 415.30f},
//         {"LA", 440.00f},
//         {"LA#", 466.16f},
//         {"SIb", 466.16f},
//         {"SI", 493.88f},
//         {"DOb", 493.88f}
//     };

//     // Check if the note is present in the map
//     if (noteFrequencies.find(note) != noteFrequencies.end()) {
//         // Adjust frequency based on the specified octave
//         float frequency = noteFrequencies[note] * std::pow(2.0f, octave - 4);
//         return frequency;
//     } else {
//         // Invalid note, return 0.0f or some default value
//         return 0.0f;
//     }
// }

GSound PVZ(0, 0.2f, false);
GSound music(0, 0.2f, false);
GSound shoot(350.0f, 0.5f, true);
GSound kill(160.0f, 1, true);
GSound damage(560.0f, 1, true);

GSound *sounds[5]{&PVZ, &music, &shoot, &kill, &damage};

double timeSound;
float BaseAmplitude = 1;
float freq = 554.37f;

double shootSound;
float shootAmp = 0;
float shootFreq = 440;

float freqlist[16 * 6]{
    554.37f, 587.33f, 554.37f, 587.33f, 466.16f, 392.00f, 392.00f, 392.00f, 466.16f, 392.00f, 392.00f, 392.00f, 587.33f, 392.00f, 392.00f, 392.00f,
    554.37f, 587.33f, 554.37f, 587.33f, 466.16f, 392.00f, 392.00f, 392.00f, 466.16f, 392.00f, 392.00f, 392.00f, 587.33f, 392.00f, 392.00f, 392.00f,
    554.37f, 587.33f, 554.37f, 587.33f, 466.16f, 392.00f, 392.00f, 392.00f, 466.16f, 392.00f, 392.00f, 392.00f, 587.33f, 392.00f, 392.00f, 392.00f,
    440.00f, 466.16f, 440.00f, 466.16f, 392.00f, 293.66f, 293.66f, 293.66f, 277.18f, 293.66f, 392.00f, 466.16f, 554.37f, 587.33f, 783.99f, 932.33f,
    554.37f, 587.33f, 554.37f, 587.33f, 466.16f, 392.00f, 392.00f, 392.00f, 466.16f, 392.00f, 392.00f, 392.00f, 587.33f, 392.00f, 392.00f, 392.00f,
    440.00f, 466.16f, 440.00f, 466.16f, 392.00f, 293.66f, 293.66f, 293.66f, 277.18f, 293.66f, 392.00f, 466.16f, 554.37f, 587.33f, 783.99f, 932.33f};
void audio_callback(float *buffer, int num_frames, int num_channels)
{
    for (int frame = 0; frame < num_frames; frame++)
    {
        float sample = 0;
        buffer[frame] = 0;
        for (auto sound : sounds)
        {
            if (sound->clip)
            {
                sample += sound->clip->samples[sound->curentClipFrame];
                sound->curentClipFrame++;
            }
            else
            {
                if (sound->AmpDecrease)
                    sound->Amp -= 0.0001f;
                if (sound->Amp < 0)
                    sound->Amp = 0;

                buffer[frame] += sinf(sound->Time * 2 * M_PI) * sound->Amp * sound->Volume;
                sound->Time += 1.0 / 44100 * sound->Freq;
            }
        }
    }
}

int main()
{
    PVZ.Amp = 0;
    Clip musicClip = LoadSoundClip("ressources/Olive.wav");
    music.Amp = 1;
    music.clip = &musicClip;
    bool menu = true;
    bool game = false;

    Window myWindow = Window();
    mfb_set_keyboard_callback(myWindow.window, OnKeyboardEvent);

    Player player;
    int fireRate = player.fireRate + 1;
    int enemySpawnRate = 45;
    int enemySpawn = 0;
    int rockSpawnRate = 120;
    int rockSpawn = 0;
    IMG font("ressources/font_map.png");
    IMG rat("ressources/Rat.png");
    IMG rock("ressources/Rock.png");
    IMG heart("ressources/Heart.png");
    Entity enemies[30];
    Entity rocks[30];

    float enemyScale = 2.5f;
    int enemyCounter = 0;
    float rockScale = 2.5f;
    int rockCounter = 0;
    int score = 0;
    int HP = 5;

    saudio_desc desc = {0};
    desc.logger.func = slog_func;
    desc.stream_cb = audio_callback;
    saudio_setup(desc);

    int note = 0;
    int timer = 0;
    do
    {
        if (IsKeyDown(KB_KEY_ESCAPE))
            break;
        timer++;

        if (timer >= 15)
        {
            note++;
            if (note >= 16 * 6)
            {
                note = 0;
            }
            PVZ.Freq = freqlist[note];
            timer = 0;
        }

        myWindow.DrawWholeWindow(0xFF3A75C5);
        if (menu)
        {
            myWindow.DrawFullRect(WINDOW_WIDTH, 100, 0, 40, BLACK);
            myWindow.DrawText("SPACE BUM", WINDOW_WIDTH / 2, 50);
            myWindow.DrawText("!!!          VS          !!!", WINDOW_WIDTH / 2, 75);
            myWindow.DrawText("SPACE RAT INVASION", WINDOW_WIDTH / 2, 105);

            myWindow.DrawFullRect(WINDOW_WIDTH, 100, 0, 300, BLACK);
            myWindow.DrawText("Press ENTER to start", WINDOW_WIDTH / 2, 340);
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
                player.x -= 10;
            if ((IsKeyDown(KB_KEY_D) || IsKeyDown(KB_KEY_RIGHT)) && player.x + player.img.width * player.Scale < WINDOW_WIDTH)
                player.x += 10;
            if ((IsKeyDown(KB_KEY_W) || IsKeyDown(KB_KEY_UP)) && player.y > 0)
                player.y -= 10;
            if ((IsKeyDown(KB_KEY_S) || IsKeyDown(KB_KEY_DOWN)) && player.y + player.img.height * player.Scale < WINDOW_HEIGHT)
                player.y += 10;
            if (WasKeyJustPressed(KB_KEY_SPACE) && fireRate >= player.fireRate)
            {
                player.SpawnBullet(player.x, player.y);
                shoot.Amp = 1;
                fireRate = 0;
            }
            enemySpawn++;
            if (enemySpawn >= enemySpawnRate && enemyCounter < 100)
            {
                enemies[enemyCounter] = Entity(rand() % WINDOW_WIDTH - rat.width * enemyScale / 2, -100, rat, enemyScale, 4, 0);
                enemySpawn = 0;
                enemyCounter++;
            }
            rockSpawn++;
            if (rockSpawn >= rockSpawnRate && rockCounter < 100)
            {
                rocks[rockCounter] = Entity(rand() % WINDOW_WIDTH - rock.width * rockScale / 2, -100, rock, rockScale, 2, (float)(rand() % 300) / 100);
                rockSpawn = 0;
                rockCounter++;
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
                        kill.Amp = 1;
                        enemies[i] = enemies[enemyCounter - 1];
                        enemyCounter--;
                        score += 100;

                        break;
                    }
                }

                if (enemies[i].Y > WINDOW_HEIGHT)
                {
                    enemies[i] = enemies[enemyCounter - 1];
                    enemyCounter--;
                    HP -= 1;
                    damage.Amp = 1;
                }
                enemies[i].Draw(myWindow);
            }

            for (int i = 0; i < rockCounter; i++)
            {
                if (player.x < rocks[i].X + rocks[i].img.width * rocks[i].Scale &&
                    player.x + player.img.width * player.Scale > rocks[i].X &&
                    player.y < rocks[i].Y + rocks[i].img.height * rocks[i].Scale &&
                    player.y + player.img.height * player.Scale > rocks[i].Y)
                {
                    damage.Amp = 1;
                    rocks[i] = rocks[rockCounter - 1];
                    rockCounter--;
                    HP -= 1;

                    break;
                }
                if (rocks[i].Y > WINDOW_HEIGHT)
                {
                    rocks[i] = rocks[rockCounter - 1];
                    rockCounter--;
                }
                rocks[i].Draw(myWindow);
            }
            myWindow.DrawFullRect(WINDOW_WIDTH, 50, 0, 0, BLACK);
            myWindow.DrawZoomedIMG(heart, 0, 0, 1.6f);
            myWindow.DrawText("= " + std::to_string(HP), 72, 10);
            myWindow.DrawText("score = " + std::to_string(score), WINDOW_WIDTH / 2, 10);
            player.Draw(myWindow);
            if (HP <= 0)
            {
                game = false;
            }
        }
        else // GAME OVER
        {
            myWindow.DrawFullRect(WINDOW_WIDTH, 150, 0, WINDOW_HEIGHT / 2 - 75, BLACK);
            myWindow.DrawText("! GAME OVER !", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 45);
            myWindow.DrawText("score = " + std::to_string(score), WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 15);
            myWindow.DrawText("Press ENTER to play again", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 15);
            if (IsKeyDown(KB_KEY_ENTER))
            {
                game = true;
                score = 0;
                HP = 5;
                player = Player();
                for (int i = 0; i < enemyCounter; i++)
                {
                    enemies[i] = enemies[29];
                }
                enemyCounter = 0;
                for (int i = 0; i < rockCounter; i++)
                {
                    rocks[i] = rocks[29];
                }
                rockCounter = 0;
            }
        }

        myWindow.SetState();

        if (myWindow.GetState() < 0)
        {
            myWindow.window = NULL;
            break;
        }
    } while (mfb_wait_sync(myWindow.window));
}