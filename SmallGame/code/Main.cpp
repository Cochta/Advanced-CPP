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
#include <fstream>

#define __cdecl

#define ARR_LEN(arr) ((int)(sizeof(arr) / sizeof(*arr)))
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

#define xstr(a) str(a)
#define str(a) #a
#define LOG(message) printf(message "(" __FILE__ ": " str(__LINE__) ")\n")

#define TILE_PX 50

#ifdef __EMSCRIPTEN__
mfb_timer *webTimer = mfb_timer_create();
double last_rendered_frame_time = -1000;
#endif

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
enum Direction
{
    Up,
    Down,
    Left,
    Right,
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

GSound PVZ(0, 0.1f, false);
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
    FILE *f = fopen("levels/User.lvl", "wb");
    fwrite(ser.buffer, ser.bufferUsed, 1, f);
    fclose(f);
}
void LoadLevel(const char *fileName)
{
    LoadedFile file = LoadFile(fileName);
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

bool menu = true;
bool selection = false;
bool game = false;
bool editor = false;

bool grounded = false;

float value = 0;

void HandleCollision(int x, int y, float &speed, Direction dir)
{
    if (tiles[y * 16 + x] == 1 || tiles[y * 16 + x] == 3)
    {
        if (dir == Down || dir == Up)
        {
            playerY = y * TILE_PX + (dir == Down ? -30 : TILE_PX);
        }
        else
        {
            playerX = x * TILE_PX + (dir == Right ? -30 : TILE_PX);
        }

        speed = 0;
        if (dir == Down)
        {
            grounded = true;
        }
    }
    else if (tiles[y * 16 + x] == 2)
    {
        playerX = LVLplayerX;
        playerY = LVLplayerY;
        damage.Amp = 1;
    }
    else if (tiles[y * 16 + x] == 5)
    {
        // selection = true;
        game = false;
        selection = true;
        shoot.Amp = 1;
    }
    if (speed > 0 && dir == Down)
    {
        grounded = false;
    }
}

float jumpspeed = 13;
float fallingSpeed = 8;

uint8_t *frameAllocMemory;
size_t frameAllocUsed = 0;
size_t frameAllocCapacity = 10000;

void FrameAllocInit()
{
    frameAllocMemory = (uint8_t *)malloc(frameAllocCapacity);
}
void *FrameAlloc(size_t size)
{
    if (frameAllocUsed + size > frameAllocCapacity)
        exit(1);

    uint8_t *allocation = frameAllocMemory + frameAllocUsed;
    frameAllocUsed += size;
    return allocation;
}
void FrameAllocTick()
{
    frameAllocUsed = 0;
}
char *GetScoreStr(int score)
{
    // char *buffer = (char *)malloc(100);
    char *buffer = (char *)FrameAlloc(100);
    snprintf(buffer, 100, "score: %i", score);
    return buffer;
}
char *Format(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    char *buffer = static_cast<char *>(FrameAlloc(size + 1));
    if (buffer == nullptr)
    {
        va_end(args);
        return nullptr;
    }

    vsnprintf(buffer, size + 1, format, args);

    va_end(args);

    return buffer;
}

static int text_width(mu_Font font, const char *text, int len)
{
    (void)font;
    return strlen(text) * 7 * 3;
}

static int text_height(mu_Font font)
{
    (void)font;
    return 9 * 3;
}
void SetButtonColors(mu_Context *ctx, int number)
{
    switch (number)
    {
    case 0:
        ctx->style->colors[MU_COLOR_BUTTON] = {58, 117, 197, 255};
        ctx->style->colors[MU_COLOR_BUTTONHOVER] = {78, 137, 197, 255};
        ctx->style->colors[MU_COLOR_BUTTONFOCUS] = {98, 157, 197, 255};
        break;
    case 1:
        ctx->style->colors[MU_COLOR_BUTTON] = {0, 255, 0, 255};
        ctx->style->colors[MU_COLOR_BUTTONHOVER] = {20, 235, 20, 255};
        ctx->style->colors[MU_COLOR_BUTTONFOCUS] = {40, 215, 40, 255};
        break;
    case 2:
        ctx->style->colors[MU_COLOR_BUTTON] = {255, 0, 0, 255};
        ctx->style->colors[MU_COLOR_BUTTONHOVER] = {235, 20, 20, 255};
        ctx->style->colors[MU_COLOR_BUTTONFOCUS] = {215, 40, 40, 255};
        break;
    case 4:
        ctx->style->colors[MU_COLOR_BUTTON] = {113, 113, 113, 255};
        ctx->style->colors[MU_COLOR_BUTTONHOVER] = {133, 133, 133, 255};
        ctx->style->colors[MU_COLOR_BUTTONFOCUS] = {153, 153, 153, 255};
        break;
    case 5:
        ctx->style->colors[MU_COLOR_BUTTON] = {253, 99, 255, 255};
        ctx->style->colors[MU_COLOR_BUTTONHOVER] = {253, 79, 235, 255};
        ctx->style->colors[MU_COLOR_BUTTONFOCUS] = {253, 59, 215, 255};
        break;

    default:
        break;
    }
}
char logbuf[64000];
int logbuf_updated;
static void write_log(const char *text)
{
    if (logbuf[0])
    {
        strcat(logbuf, "\n");
    }
    strcat(logbuf, text);
    logbuf_updated = 1;
}
int main()
{
    PVZ.Amp = 1;
    // Clip musicClip = LoadSoundClip("ressources/Olive.wav");
    // music.Amp = 1;
    // music.clip = &musicClip;

    bool gotPaths = false;

    Window myWindow = Window();
    mfb_set_keyboard_callback(myWindow.window, OnKeyboardEvent);
    mfb_set_mouse_move_callback(myWindow.window, OnMouseMove);
    mfb_set_mouse_button_callback(myWindow.window, OnMouseButton);

    IMG font("ressources/font_map.png");
    IMG rat("ressources/Rat.png");
    IMG rock("ressources/Rock.png");
    IMG heart("ressources/Heart.png");
    IMG beer("ressources/Beer.png");
    IMG shield("ressources/Shield.png");
    char *iconArray[5]{"0", "x", "0", ">", "<"};

    saudio_desc desc = {0};
    desc.logger.func = slog_func;
    desc.stream_cb = audio_callback;
    saudio_setup(desc);

    int note = 0;
    int timer = 0;

    int jumpTimer = 30;
    FrameAllocInit();

    printf(Format("banane", "carot", "olive"));
    mu_Context *ctx = (mu_Context *)malloc(sizeof(mu_Context));
    mu_init(ctx);
    ctx->text_width = text_width;
    ctx->text_height = text_height;
    // ctx->style->colors[MU_COLOR_BUTTON] = {75, 255, 75, 255};
    int actualColor = Empty;
    do
    {

#ifdef __EMSCRIPTEN__
        double now_time = mfb_timer_now(webTimer);
        double delta_time = now_time - last_rendered_frame_time;
        double target_delta_time = 1.f / 60.f;
        if (delta_time < target_delta_time)
        {
            mfb_wait_sync(myWindow.window);
            continue;
        }
        last_rendered_frame_time = now_time;
#else
        mfb_set_target_fps(60);
#endif
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

        mu_input_mousemove(ctx, mouseX, mouseY);
        //mu_input_keydown(ctx, MU_KEY_ALT);
        if (WasMouseJustPressed(MOUSE_LEFT))
        {
            mu_input_mousedown(ctx, mouseX, mouseY, 1);
        }
        if (WasMouseJustReleased(MOUSE_LEFT))
        {
            mu_input_mouseup(ctx, mouseX, mouseY, 1);
        }

        mu_begin(ctx);
        if (mu_begin_window(ctx, "My Window", mu_rect(0, 0, 500, 500)))
        {
            if (mu_header(ctx, "Window Info"))
            {
                mu_Container *win = mu_get_current_container(ctx);
                char buf[64];
                int a[3]{200, -1};
                mu_layout_row(ctx, 2, a, 0);
                mu_label(ctx, "Position:");
                sprintf(buf, "%d, %d", win->rect.x, win->rect.y);
                mu_label(ctx, buf);
                mu_label(ctx, "Size:");
                sprintf(buf, "%d, %d", win->rect.w, win->rect.h);
                mu_label(ctx, buf);
                mu_button(ctx, "Banane");
            }

            mu_end_window(ctx);
        }
        mu_end(ctx);

#pragma region
        if (menu)
        {
            myWindow.DrawFullRect(WINDOW_WIDTH, 100, 0, 40, BLACK);
            myWindow.DrawText("!!! PIXEL PLATEFORMER !!!", WINDOW_WIDTH / 2, 75, PivotType::Center);
            uint32_t selectcolor = BLACK;

            if (mouseX >= 0 && mouseX <= WINDOW_WIDTH &&
                mouseY >= 300 && mouseY <= 400)
            {
                selectcolor = GREEN;
                if (WasMouseJustReleased(MOUSE_BTN_1))
                {
                    menu = false;
                    selection = true;
                }
            }

            myWindow.DrawFullRect(WINDOW_WIDTH, 100, 0, 300, selectcolor);
            myWindow.DrawText("Select a level", WINDOW_WIDTH / 2, 340, PivotType::Center);

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
            myWindow.DrawText("level Editor", WINDOW_WIDTH / 2, 540, PivotType::Center);
        }
        else if (selection)
        {
            const char *directory_path = "levels";
            int i = 0;
            int j = 0;
            for (const auto &file : std::filesystem::directory_iterator(directory_path))
            {
                std::string name = file.path().string();
                const char *path = name.c_str();
                std::string cutName = name.substr(7);
                cutName = cutName.substr(0, cutName.length() - 4);
                // mousclickdetection
                if (i == 6)
                {
                    i = 0;
                    j += 1;
                }
                uint32_t lvlcolor = BLACK;
                if (mouseX >= 29 + i * 29 + 100 * i && mouseX <= 29 + i * 29 + 100 * i + 100 &&
                    mouseY >= 29 + j * 29 + 100 * j && mouseY <= 29 + j * 29 + 100 * j + 100)
                {
                    lvlcolor = GREEN;
                    if (WasMouseJustReleased(MOUSE_BTN_1))
                    {
                        LoadLevel(path);
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
                        game = true;
                        selection = false;
                    }
                }

                myWindow.DrawFullRect(100, 100, 29 + i * 29 + 100 * i, 29 + j * 29 + 100 * j, lvlcolor);
                myWindow.DrawText(cutName, 29 + 42 + i * 29 + 100 * i, 29 + 34 + j * 29 + 100 * j, PivotType::Center);
                i++;
            }
        }
        else if (game)
        {
            jumpTimer++;

            if (IsKeyDown(KB_KEY_D))
            {
                float movespeed = 5;
                for (int y = 0; y < 16; y++)
                {
                    for (int x = 0; x < 16; x++)
                    {
                        if (playerX + 30 + movespeed > x * TILE_PX && playerX < x * TILE_PX + TILE_PX &&
                            playerY + 30 > y * TILE_PX && playerY < y * TILE_PX + TILE_PX)
                        {
                            HandleCollision(x, y, movespeed, Right);
                        }
                    }
                }
                playerX += movespeed;
            }
            if (IsKeyDown(KB_KEY_A))
            {
                float movespeed = 5;
                for (int y = 0; y < 16; y++)
                {
                    for (int x = 0; x < 16; x++)
                    {
                        if (playerX - movespeed < x * TILE_PX + TILE_PX && playerX + 30 > x * TILE_PX &&
                            playerY + 30 > y * TILE_PX && playerY < y * TILE_PX + TILE_PX)
                        {
                            HandleCollision(x, y, movespeed, Left);
                        }
                    }
                }
                playerX -= movespeed;
            }
            if (jumpTimer >= 13)
            {
                fallingSpeed += 1;
                for (int y = 0; y < 16; y++)
                {
                    for (int x = 0; x < 16; x++)
                    {
                        if (playerY + 30 + fallingSpeed > y * TILE_PX && playerY < y * TILE_PX &&
                            playerX + 30 > x * TILE_PX && playerX < x * TILE_PX + TILE_PX)
                        {
                            HandleCollision(x, y, fallingSpeed, Down);
                        }
                    }
                }

                playerY += fallingSpeed;
            }
            else
            {
                if (jumpTimer == 12)
                {
                    fallingSpeed = 0;
                }
                if (WasKeyJustReleased(KB_KEY_SPACE))
                {
                    jumpTimer = 13;
                }

                jumpspeed -= 1;

                for (int y = 0; y < 16; y++)
                {
                    for (int x = 0; x < 16; x++)
                    {
                        if (playerY - jumpspeed < y * TILE_PX + TILE_PX && playerY + 30 > y * TILE_PX &&
                            playerX + 30 > x * TILE_PX && playerX < x * TILE_PX + TILE_PX)
                        {
                            HandleCollision(x, y, jumpspeed, Up);
                        }
                    }
                }
                playerY -= jumpspeed;
            }

            if (WasKeyJustPressed(KB_KEY_SPACE) && jumpTimer >= 13 && grounded)
            {
                jumpspeed = 17;
                jumpTimer = 0;
                grounded = false;
                kill.Amp = 1;
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
                    else if (type == Tiles::Goal)
                    {
                        myWindow.DrawFullRect(TILE_PX, TILE_PX, x * TILE_PX, y * TILE_PX, 0xFFFD63FF);
                    }
                }
            }

            myWindow.DrawFullRect(30, 30, playerX, playerY, grounded ? 0xFFACACAC : BLUE);
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
                LoadLevel("levels/User.lvl");
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
                        if (IsMouseDown(MOUSE_LEFT))
                        {
                            tiles[y * 16 + x] = actualColor;
                            modif = true;
                        }
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
            if (modif && (WasMouseJustReleased(MOUSE_LEFT) || WasKeyJustReleased(KB_KEY_1) || WasKeyJustReleased(KB_KEY_2) || WasKeyJustReleased(KB_KEY_3) || WasKeyJustReleased(KB_KEY_4) || WasKeyJustReleased(KB_KEY_5)))
            {
                HistoryCommit();
                modif = false;
            }
            myWindow.DrawFullRect(40, 40, 55, WINDOW_HEIGHT - 45, 0xFF3A75C5);
            myWindow.DrawText("1", 55 + 13, WINDOW_HEIGHT - 40, PivotType::Center);

            myWindow.DrawFullRect(40, 40, 55 + 50, WINDOW_HEIGHT - 45, GREEN);
            myWindow.DrawText("2", 55 + 13 + 50, WINDOW_HEIGHT - 40, PivotType::Center);

            myWindow.DrawFullRect(40, 40, 55 + 50 * 2, WINDOW_HEIGHT - 45, RED);
            myWindow.DrawText("3", 55 + 13 + 50 * 2, WINDOW_HEIGHT - 40, PivotType::Center);

            myWindow.DrawFullRect(40, 40, 55 + 50 * 3, WINDOW_HEIGHT - 45, 0xFFFD63FF);
            myWindow.DrawText("4", 55 + 13 + 50 * 3, WINDOW_HEIGHT - 40, PivotType::Center);

            myWindow.DrawFullRect(40, 40, 55 + 50 * 4, WINDOW_HEIGHT - 45, 0xFFACACAC);
            myWindow.DrawText("5", 55 + 13 + 50 * 4, WINDOW_HEIGHT - 40, PivotType::Center);

            mu_begin(ctx);
            if (mu_begin_window_ex(ctx, "", mu_rect(WINDOW_WIDTH - 90, 0, 90, 315), MU_OPT_NORESIZE | MU_OPT_NOSCROLL | MU_OPT_NOINTERACT))
            {
                int a[1]{50};
                char *s[6]{"1", "2", "3", "6", "4", "5"};
                mu_Container *win = mu_get_current_container(ctx);
                mu_layout_row(ctx, 1, nullptr, 0);
                for (int i = 0; i < 6; i++)
                {
                    if (i == 3)
                    {
                        i++;
                    }

                    SetButtonColors(ctx, i);
                    if (mu_button(ctx, s[i]))
                    {
                        actualColor = i;
                    }
                }
                // ctx->style = ;

                // static char buf[128];
                int submitted = 0;

                // if (mu_textbox(ctx, buf, sizeof(buf)) & MU_RES_SUBMIT)
                // {
                //     submitted = 1;
                // }
                ctx->style->colors[MU_COLOR_BUTTON] = {75, 75, 75, 255};
                ctx->style->colors[MU_COLOR_BUTTONHOVER] = {95, 95, 95, 255};
                ctx->style->colors[MU_COLOR_BUTTONFOCUS] = {115, 115, 115, 255};
                if (mu_button(ctx, "Save"))
                {
                    submitted = 1;
                }
                if (submitted)
                {
                    SaveLevel();
                    ;
                    // buf[0] = '\0';
                }

                mu_end_window(ctx);
            }

            mu_end(ctx);
        }
#pragma endregion
        /* render */
        mu_Command *cmd = NULL;
        while (mu_next_command(ctx, &cmd))
        {
            switch (cmd->type)
            {
            case MU_COMMAND_TEXT:
                myWindow.DrawText(cmd->text.str, cmd->text.pos.x, cmd->text.pos.y, PivotType::TopLeft);
                break;
            case MU_COMMAND_RECT:
                myWindow.DrawFullRect(cmd->rect.rect.w, cmd->rect.rect.h, cmd->rect.rect.x, cmd->rect.rect.y,
                                      MFB_ARGB(cmd->rect.color.a, cmd->rect.color.r, cmd->rect.color.g, cmd->rect.color.b));
                break;
            case MU_COMMAND_ICON:
                myWindow.DrawIMG(rat, cmd->rect.rect.x, cmd->rect.rect.y);
                // myWindow.DrawText(iconArray[cmd->icon.id], cmd->rect.rect.x, cmd->rect.rect.y, PivotType::TopLeft);
                break;
            }
        }
        TickInput();
        myWindow.SetState();
        FrameAllocTick();

        if (myWindow.GetState() < 0)
        {
            myWindow.window = NULL;
            break;
        }
    } while (mfb_wait_sync(myWindow.window));
}