#include <iostream>
#include "stdio.h"
#include "string.h"
#include <sstream>
#include "Player.h"
#include "Entity.h"
#define _USE_MATH_DEFINES
#include "math.h"
#include "GSound.h"
#include <filesystem>
// #include <unordered_map>

#define ARR_LEN(arr) ((int)(sizeof(arr) / sizeof(*arr)))
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

#define xstr(a) str(a)
#define str(a) #a
#define LOG(message) printf(message "(" __FILE__ ": " str(__LINE__) ")\n")

#define TILE_PX 50

enum Tiles
{
    Empty = 0,
    Floor = 1,
    Danger = 2,
    Wall = 3,
    Player = 4,
    Goal = 5
};

enum Mod
{
    Save,
    Load
};

bool keys_state[KB_KEY_LAST + 1];
bool keys_state_prev[KB_KEY_LAST + 1];
bool mouseButtonState[8];
bool mouseButtonStatePrev[8];

int mouseX;
int mouseY;

void TickInput()
{
    memcpy(keys_state_prev, keys_state, sizeof(keys_state));
    memcpy(mouseButtonStatePrev, mouseButtonState, sizeof(mouseButtonState));
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
bool IsMouseDown(mfb_mouse_button button)
{
    return mouseButtonState[button];
}
bool WasMouseJustPressed(mfb_mouse_button button)
{
    return !mouseButtonStatePrev[button] && mouseButtonState[button];
}
bool WasMouseJustReleased(mfb_mouse_button button)
{
    return mouseButtonStatePrev[button] && !mouseButtonState[button];
}
void OnKeyboardEvent(struct mfb_window *window, mfb_key key, mfb_key_mod mod, bool isPressed)
{
    (void)window;
    (void)mod;
    keys_state[key] = isPressed;
}
void OnMouseMove(struct mfb_window *window, int x, int y)
{
    mouseX = x;
    mouseY = y;
}
void OnMouseButton(struct mfb_window *window, mfb_mouse_button button, mfb_key_mod mod, bool isPressed)
{
    mouseButtonState[button] = isPressed;
}

GSound PVZ(0, 0.2f, false);
// GSound music(0, 0.2f, false);
GSound shoot(350.0f, 0.5f, true);
GSound kill(160.0f, 1, true);
GSound damage(560.0f, 1, true);

GSound *sounds[4]{&PVZ, &shoot, &kill, &damage};

int tiles[16 * 16];

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
struct Serializer
{
    Mod mod;
    int bufferCapacity;
    int bufferUsed;
    uint8_t *buffer;
};
void Serialize(Serializer *ser, void *ptr, size_t size)
{
    if (ser->mod == Mod::Load)
    {
        size_t remainingBytes = ser->bufferCapacity - ser->bufferUsed;
        if (remainingBytes < size)
        {
            printf("Serializer tried to read past end of buffer");
            exit(-1);
        }

        memcpy(ptr, ser->buffer + ser->bufferUsed, size);
        ser->bufferUsed += size;
    }
    else
    {
        size_t remainingCapacity = ser->bufferCapacity - ser->bufferUsed;
        if (remainingCapacity < size)
        {
            printf("out of memory");
            exit(-1);
        }

        memcpy(ser->buffer + ser->bufferUsed, ptr, size);
        ser->bufferUsed += size;
    }
}
void SerializeLevel(Serializer *ser)
{
    Serialize(ser, tiles, sizeof(tiles));
}
void SaveLevel()
{
    Serializer ser = {};
    ser.mod = Mod::Save;
    ser.bufferCapacity = 16 * 16 * 4;
    ser.buffer = (uint8_t *)malloc(ser.bufferCapacity);
    SerializeLevel(&ser);
    FILE *f = fopen("levels/Test.lvl", "wb");
    fwrite(ser.buffer, ser.bufferUsed, 1, f);
    fclose(f);
}
void LoadLevel()
{
    LoadedFile file = LoadFile("levels/Test.lvl");
    Serializer ser = {};
    ser.mod = Mod::Load;
    ser.buffer = file.data;
    ser.bufferCapacity = file.size;
    ser.bufferUsed = 0;
    SerializeLevel(&ser);
}

Serializer historySteps[256];
int historyStepCount = 0;
bool modif = false;
void HistoryCommit()
{
    Serializer ser = {};
    ser.mod = Mod::Save;
    ser.bufferCapacity = 16 * 16 * 4;
    ser.buffer = (uint8_t *)malloc(ser.bufferCapacity);
    SerializeLevel(&ser);
    historySteps[historyStepCount] = ser;
    historyStepCount++;
}
void Undo()
{
    Serializer ser;
    if (historyStepCount <= 1)
    {
        return;
    }
    ser = historySteps[historyStepCount - 2];
    ser.mod = Mod::Load;
    ser.bufferCapacity = ser.bufferUsed;
    ser.bufferUsed = 0;
    SerializeLevel(&ser);
    historyStepCount--;
}
int playerX;
int playerY;
int LVLplayerX;
int LVLplayerY;
int goalX;
int goalY;

bool grounded = false;

int main()
{
    PVZ.Amp = 0;
    // Clip musicClip = LoadSoundClip("ressources/Olive.wav");
    // music.Amp = 1;
    // music.clip = &musicClip;
    bool menu = true;
    bool selection = false;
    bool game = false;
    bool editor = false;

    bool gotPaths = false;

    Window myWindow = Window();
    mfb_set_keyboard_callback(myWindow.window, OnKeyboardEvent);
    mfb_set_mouse_move_callback(myWindow.window, OnMouseMove);
    mfb_set_mouse_button_callback(myWindow.window, OnMouseButton);

    IMG font("ressources/font_map.png");
    IMG rat("ressources/Rat.png");
    IMG rock("ressources/Rock.png");
    IMG heart("ressources/Heart.png");

    saudio_desc desc = {0};
    desc.logger.func = slog_func;
    desc.stream_cb = audio_callback;
    saudio_setup(desc);

    int note = 0;
    int timer = 0;

    int jumpTimer = 30;
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
            myWindow.DrawText("!!! PIXEL PLATEFORMER !!!", WINDOW_WIDTH / 2, 75);
            uint32_t selectcolor = BLACK;

            if (mouseX >= 0 && mouseX <= WINDOW_WIDTH &&
                mouseY >= 300 && mouseY <= 400)
            {
                selectcolor = GREEN;
                if (WasMouseJustReleased(MOUSE_BTN_1))
                {
                    LoadLevel();
                    for (int y = 0; y < 16; y++)
                    {
                        for (int x = 0; x < 16; x++)
                        {
                            if (tiles[y * 16 + x] == Tiles::Goal)
                            {
                                goalX = x;
                                goalY = y;
                            }
                            if (tiles[y * 16 + x] == Tiles::Player)
                            {
                                playerX = x * TILE_PX + 10;
                                playerY = y * TILE_PX + 10;
                                LVLplayerX = playerX;
                                LVLplayerY = playerY;
                            }
                        }
                    }
                    menu = false;
                    game = true;
                    editor = false;
                    selection = false;
                }
            }

            myWindow.DrawFullRect(WINDOW_WIDTH, 100, 0, 300, selectcolor);
            myWindow.DrawText("Select a level", WINDOW_WIDTH / 2, 340);

            uint32_t editorcolor = BLACK;
            if (mouseX >= 0 && mouseX <= WINDOW_WIDTH &&
                mouseY >= 500 && mouseY <= 600)
            {
                editorcolor = GREEN;
                if (WasMouseJustReleased(MOUSE_BTN_1))
                {
                    for (int y = 0; y < 16; y++)
                    {
                        for (int x = 0; x < 16; x++)
                        {
                            tiles[y * 16 + x] = 0;
                        }
                    }
                    historyStepCount = 0;
                    HistoryCommit();
                    menu = false;
                    game = false;
                    editor = true;
                }
            }

            myWindow.DrawFullRect(WINDOW_WIDTH, 100, 0, 500, editorcolor);
            myWindow.DrawText("level Editor", WINDOW_WIDTH / 2, 540);
        }
        // else if (selection)
        // {
        //     if (!gotPaths)
        //     {
        //         const char* directory_path = "levels";
        //         for (const auto &file : std::filesystem::directory_iterator(directory_path))
        //         {
        //             std::string name = file.path();
        //             printf(name.substr(8,5));
        //         }
        //         gotPaths = true;
        //     }
        // }
        else if (game)
        {
            jumpTimer++;

            if (jumpTimer >= 13)
            {
                int FallingSpeed = 4;
                for (int y = 0; y < 16; y++)
                {
                    for (int x = 0; x < 16; x++)
                    {
                        if (tiles[y * 16 + x] == 1 || tiles[y * 16 + x] == 3)
                        {
                            if (playerY + 30 + FallingSpeed > y * TILE_PX && playerY < y * TILE_PX + TILE_PX &&
                                playerX + 30 > x * TILE_PX && playerX < x * TILE_PX + TILE_PX)
                            {
                                playerY = y * TILE_PX - 30;
                                FallingSpeed = 0;
                                grounded = true;
                            }
                        }
                        else if (tiles[y * 16 + x] == 2)
                        {
                            if (playerY + 30 + FallingSpeed > y * TILE_PX && playerY < y * TILE_PX + TILE_PX &&
                                playerX + 30 > x * TILE_PX && playerX < x * TILE_PX + TILE_PX)
                            {
                                playerX = LVLplayerX;
                                playerY = LVLplayerY;
                            }
                        }
                    }
                }

                playerY += FallingSpeed;
            }
            else
            {
                int jumpspeed = 10;
                for (int y = 0; y < 16; y++)
                {
                    for (int x = 0; x < 16; x++)
                    {
                        if (tiles[y * 16 + x] == 1 || tiles[y * 16 + x] == 3)
                        {
                            if (playerY - jumpspeed < y * TILE_PX + TILE_PX && playerY + 30 > y * TILE_PX &&
                                playerX + 30 > x * TILE_PX && playerX < x * TILE_PX + TILE_PX)
                            {
                                playerY = y * TILE_PX + TILE_PX;
                                jumpspeed = 0;
                            }
                        }
                        else if (tiles[y * 16 + x] == 2)
                        {
                            if (playerY - jumpspeed < y * TILE_PX + TILE_PX && playerY + 30 > y * TILE_PX &&
                                playerX + 30 > x * TILE_PX && playerX < x * TILE_PX + TILE_PX)
                            {
                                playerX = LVLplayerX;
                                playerY = LVLplayerY;
                            }
                        }
                    }
                }
                playerY -= jumpspeed;
            }
            if (IsKeyDown(KB_KEY_D))
            {
                int movespeed = 5;
                for (int y = 0; y < 16; y++)
                {
                    for (int x = 0; x < 16; x++)
                    {
                        if (tiles[y * 16 + x] == 1 || tiles[y * 16 + x] == 3)
                        {
                            if (playerX + 30 + movespeed > x * TILE_PX && playerX < x * TILE_PX + TILE_PX &&
                                playerY + 30 > y * TILE_PX && playerY < y * TILE_PX + TILE_PX)
                            {
                                playerX = x * TILE_PX - 30;
                                movespeed = 0;
                            }
                        }
                        else if (tiles[y * 16 + x] == 2)
                        {
                            if (playerX + 30 + movespeed > x * TILE_PX && playerX < x * TILE_PX + TILE_PX &&
                                playerY + 30 > y * TILE_PX && playerY < y * TILE_PX + TILE_PX)
                            {
                                playerX = LVLplayerX;
                                playerY = LVLplayerY;
                            }
                        }
                    }
                }
                playerX += movespeed;
            }
            if (IsKeyDown(KB_KEY_A))
            {
                int movespeed = 5;
                for (int y = 0; y < 16; y++)
                {
                    for (int x = 0; x < 16; x++)
                    {
                        if (tiles[y * 16 + x] == 1 || tiles[y * 16 + x] == 3)
                        {
                            if (playerX - movespeed < x * TILE_PX + TILE_PX && playerX + 30 > x * TILE_PX &&
                                playerY + 30 > y * TILE_PX && playerY < y * TILE_PX + TILE_PX)
                            {
                                playerX = x * TILE_PX + TILE_PX;
                                movespeed = 0;
                            }
                            else if (tiles[y * 16 + x] == 2)
                            {
                                if (playerX - movespeed < x * TILE_PX + TILE_PX && playerX + 30 > x * TILE_PX &&
                                    playerY + 30 > y * TILE_PX && playerY < y * TILE_PX + TILE_PX)
                                {
                                    playerX = LVLplayerX;
                                    playerY = LVLplayerY;
                                }
                            }
                        }
                    }
                }
                playerX -= movespeed;
            }
            if (WasKeyJustPressed(KB_KEY_SPACE) && jumpTimer >= 13 && grounded)
            {
                jumpTimer = 0;
                grounded = false;
            }
            if (WasKeyJustPressed(KB_KEY_BACKSPACE))
            {
                menu = true;
                game = false;
                editor = false;
            }
            int floor[16 * 16];
            for (int y = 0; y < 16; y++) // draw tile
            {
                for (int x = 0; x < 16; x++)
                {
                    int type = tiles[y * 16 + x];

                    if (type == Tiles::Floor)
                    {
                        myWindow.DrawFullRect(TILE_PX, TILE_PX, x * TILE_PX, y * TILE_PX, GREEN);
                    }
                    else if (type == Tiles::Danger)
                    {
                        myWindow.DrawFullRect(TILE_PX, TILE_PX, x * TILE_PX, y * TILE_PX, RED);
                    }
                    else if (type == Tiles::Wall)
                    {
                        myWindow.DrawFullRect(TILE_PX, TILE_PX, x * TILE_PX, y * TILE_PX, BLACK);
                    }
                }
            }
            myWindow.DrawFullRect(30, 30, playerX, playerY, WHITE);
            myWindow.DrawFullRect(30, 30, goalX * TILE_PX + 10, goalY * TILE_PX + 10, 0xFFFD63FF);
        }
        else if (editor)
        {
            if (WasKeyJustPressed(KB_KEY_BACKSPACE))
            {
                menu = true;
                game = false;
                editor = false;
            }
            if (WasKeyJustPressed(KB_KEY_S))
            {
                SaveLevel();
            }
            if (WasKeyJustPressed(KB_KEY_L))
            {
                LoadLevel();
                HistoryCommit();
            }
            if (WasKeyJustPressed(KB_KEY_Y) && IsKeyDown(KB_KEY_LEFT_CONTROL))
            {
                if (historyStepCount <= 0)
                    printf("nothing to undo");
                else
                    Undo();
            }
            for (int y = 0; y < 16; y++) // draw tiles
            {
                for (int x = 0; x < 16; x++)
                {
                    int type = tiles[y * 16 + x];

                    if (type == Tiles::Floor)
                    {
                        myWindow.DrawFullRect(TILE_PX, TILE_PX, x * TILE_PX, y * TILE_PX, GREEN);
                    }
                    else if (type == Tiles::Danger)
                    {
                        myWindow.DrawFullRect(TILE_PX, TILE_PX, x * TILE_PX, y * TILE_PX, RED);
                    }
                    else if (type == Tiles::Wall)
                    {
                        myWindow.DrawFullRect(TILE_PX, TILE_PX, x * TILE_PX, y * TILE_PX, BLACK);
                    }
                    else if (type == Tiles::Player)
                    {
                        myWindow.DrawFullRect(30, 30, x * TILE_PX + 10, y * TILE_PX + 10, 0xFFACACAC);
                    }
                    else if (type == Tiles::Goal)
                    {
                        myWindow.DrawFullRect(30, 30, x * TILE_PX + 10, y * TILE_PX + 10, 0xFFFD63FF);
                    }

                    if (mouseX >= x * TILE_PX && mouseX < (x * TILE_PX) + TILE_PX &&
                        mouseY >= y * TILE_PX && mouseY < (y * TILE_PX) + TILE_PX)
                    {
                        myWindow.DrawFullRect(TILE_PX, TILE_PX, x * TILE_PX, y * TILE_PX, WHITE);

                        if (IsKeyDown(KB_KEY_2))
                        {
                            tiles[y * 16 + x] = Tiles::Floor;
                            modif = true;
                        }
                        if (IsKeyDown(KB_KEY_3))
                        {
                            tiles[y * 16 + x] = Tiles::Danger;
                            modif = true;
                        }
                        if (IsKeyDown(KB_KEY_1))
                        {
                            tiles[y * 16 + x] = Tiles::Empty;
                            modif = true;
                        }
                        if (WasKeyJustReleased(KB_KEY_4))
                        {
                            for (int y = 0; y < 16; y++)
                            {
                                for (int x = 0; x < 16; x++)
                                {
                                    if (tiles[y * 16 + x] == Tiles::Goal)
                                    {
                                        tiles[y * 16 + x] = Tiles::Empty;
                                    }
                                }
                            }
                            tiles[y * 16 + x] = Tiles::Goal;
                            modif = true;
                        }
                        if (WasKeyJustReleased(KB_KEY_5))
                        {
                            for (int y = 0; y < 16; y++)
                            {
                                for (int x = 0; x < 16; x++)
                                {
                                    if (tiles[y * 16 + x] == Tiles::Player)
                                    {
                                        tiles[y * 16 + x] = Tiles::Empty;
                                    }
                                }
                            }
                            tiles[y * 16 + x] = Tiles::Player;
                            modif = true;
                        }
                    }
                    if (x == 15 || x == 0 || y == 0 || y == 15)
                    {
                        tiles[y * 16 + x] = Tiles::Wall;
                    }
                }
            }
            if (modif && (WasKeyJustReleased(KB_KEY_1) || WasKeyJustReleased(KB_KEY_2) || WasKeyJustReleased(KB_KEY_3) || WasKeyJustReleased(KB_KEY_4) || WasKeyJustReleased(KB_KEY_5)))
            {
                HistoryCommit();
                printf("commit");
                modif = false;
            }
            myWindow.DrawFullRect(40, 40, 55, WINDOW_HEIGHT - 45, 0xFF3A75C5);
            myWindow.DrawText("1", 55 + 13, WINDOW_HEIGHT - 40);

            myWindow.DrawFullRect(40, 40, 55 + 50, WINDOW_HEIGHT - 45, GREEN);
            myWindow.DrawText("2", 55 + 13 + 50, WINDOW_HEIGHT - 40);

            myWindow.DrawFullRect(40, 40, 55 + 50 * 2, WINDOW_HEIGHT - 45, RED);
            myWindow.DrawText("3", 55 + 13 + 50 * 2, WINDOW_HEIGHT - 40);

            myWindow.DrawFullRect(40, 40, 55 + 50 * 3, WINDOW_HEIGHT - 45, 0xFFFD63FF);
            myWindow.DrawText("4", 55 + 13 + 50 * 3, WINDOW_HEIGHT - 40);

            myWindow.DrawFullRect(40, 40, 55 + 50 * 4, WINDOW_HEIGHT - 45, 0xFFACACAC);
            myWindow.DrawText("5", 55 + 13 + 50 * 4, WINDOW_HEIGHT - 40);
        }
        TickInput();
        myWindow.SetState();

        if (myWindow.GetState() < 0)
        {
            myWindow.window = NULL;
            break;
        }
    } while (mfb_wait_sync(myWindow.window));
}