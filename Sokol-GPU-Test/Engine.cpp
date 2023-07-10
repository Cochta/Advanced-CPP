#define SOKOL_IMPL
#define SOKOL_D3D11
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#include "Shader.glsl.h"
#include "IMG.cpp"
#include "Engine.hpp"
#include "Draw.cpp"

#include <cstring>

bool keys[SAPP_MAX_KEYCODES + 1];
bool previousKeys[SAPP_MAX_KEYCODES + 1];

bool mouseButtons[SAPP_MAX_MOUSEBUTTONS + 1];
bool previousMouseButtons[SAPP_MAX_MOUSEBUTTONS + 1];

Vector2F mousePosition = {0, 0};
Vector2F previousMousePosition = {0, 0};
bool mouseMoved = false;
void GameFrame();

static struct
{
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
} state;

static void init(void)
{
    IMG texture("ressources/SimpleTileset.png");

    sg_desc desc = {
        .context = sapp_sgcontext(),
        .logger.func = slog_func,
    };
    sg_setup(desc);

    state.bind.vertex_buffers[0] = sg_make_buffer((sg_buffer_desc){
        .size = sizeof(vertexBuffer),
        .usage = SG_USAGE_DYNAMIC,
        .label = "triangle-vertices"});

    state.bind.index_buffer = sg_make_buffer((sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .size = sizeof(indexBuffer),
        .usage = SG_USAGE_DYNAMIC,
        .label = "triangle-indices"});

    state.bind.fs_images[SLOT_tex] = sg_make_image((sg_image_desc){
        .width = texture.width,
        .height = texture.height,
        .data.subimage[0][0] = sg_range{texture.img, (size_t)(texture.width * texture.height * texture.channels)},
        .label = "cube-texture"});

    // create shader from code-generated sg_shader_desc
    sg_shader shd = sg_make_shader(Shader_shader_desc(sg_query_backend()));

    // create a pipeline object (default render states are fine for triangle)
    sg_pipeline_desc pd{
        .index_type = SG_INDEXTYPE_UINT32,
        .shader = shd,
        // if the vertex layout doesn't have gaps, don't need to provide strides and offsets
        .layout = {
            .attrs = {
                [ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4,
                [ATTR_vs_uv0].format = SG_VERTEXFORMAT_FLOAT2}},

        .label = "triangle-pipeline",

        // Classic alpha blending
        .colors[0].blend = (sg_blend_state){
            .enabled = true,
            .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
            .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        }};

    state.pip = sg_make_pipeline(pd);

    // a pass action to clear framebuffer to black
    state.pass_action = (sg_pass_action){
        .colors[0] = {.load_action = SG_LOADACTION_CLEAR, .clear_value = {0.0f, 0.0f, 0.0f, 1.0f}}};
}

void frame(void)
{
    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);

    ClearDrawBuffers();

    GameFrame();

    sg_update_buffer(state.bind.vertex_buffers[0], (sg_range){.ptr = vertexBuffer, .size = vertexBufferUsed * sizeof(*vertexBuffer)});
    sg_update_buffer(state.bind.index_buffer, (sg_range){.ptr = indexBuffer, .size = indexBufferUsed * sizeof(*indexBuffer)});

    sg_draw(0, indexBufferUsed, 1);
    sg_end_pass();
    sg_commit();
}

void cleanup(void)
{
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    return (sapp_desc){
        .win32_console_create = true,
        .event_cb = OnInput,
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .width = 800,
        .height = 800,
        .window_title = "Triangle (sokol-app)",
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}
void OnInput(const sapp_event *event)
{
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN)
    {
        keys[event->key_code] = true;
    }
    else if (event->type == SAPP_EVENTTYPE_KEY_UP)
    {
        keys[event->key_code] = false;
    }
    else if (event->type == SAPP_EVENTTYPE_MOUSE_DOWN)
    {
        mouseButtons[event->mouse_button] = true;
    }
    else if (event->type == SAPP_EVENTTYPE_MOUSE_UP)
    {
        mouseButtons[event->mouse_button] = false;
    }
    else if (event->type == SAPP_EVENTTYPE_MOUSE_MOVE)
    {
        previousMousePosition = Vector2F(mousePosition.X, mousePosition.Y);
        mousePosition = ConvertInputPosition({event->mouse_x, event->mouse_y});
        mouseMoved = true;
    }
}

void Update()
{
    if (!mouseMoved)
    {
        previousMousePosition = Vector2F(mousePosition.X, mousePosition.Y);
    }

    mouseMoved = false;
    memcpy(previousKeys, keys, sizeof(keys));
    memcpy(previousMouseButtons, mouseButtons, sizeof(mouseButtons));
}

bool IsKeyPressed(int key)
{
    return keys[key] && !previousKeys[key];
}

bool IsKeyReleased(int key)
{
    return !keys[key] && previousKeys[key];
}

bool IsKeyDown(int key)
{
    return keys[key];
}

bool IsMouseButtonPressed(int button)
{
    return mouseButtons[button] && previousMouseButtons[button] == 0;
}

bool IsMouseButtonReleased(int button)
{
    return mouseButtons[button] == 0 && previousMouseButtons[button];
}

bool IsMouseButtonDown(int button)
{
    return mouseButtons[button];
}

Vector2F GetMousePosition()
{
    return mousePosition;
}

Vector2F GetPreviousMousePosition()
{
    return previousMousePosition;
}
Vector2F ConvertInputPosition(Vector2F position)
{
    return {position.X, sapp_height() - position.Y};
}