//------------------------------------------------------------------------------
//  clear-sapp.c
//------------------------------------------------------------------------------
#define SOKOL_IMPL
#define SOKOL_D3D11
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_log.h"
#include "sokol_glue.h"

sg_pass_action pass_action;
int timer = 0;
int nbframes = 60;
void init(void)
{
    sg_desc desc = {
        .context = sapp_sgcontext(),
        .logger.func = slog_func,
    };
    sg_setup(desc);
    pass_action = (sg_pass_action){
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = {1.0f, 1.0f, 1.0f, 1.0f}}};
}

void frame(void)
{
    timer++;
    if (timer <= nbframes)
    {
        float r = pass_action.colors[0].clear_value.r + 0.01f;
        pass_action.colors[0].clear_value.r = (r > 1.0f) ? 0.0f : r;
    }
    else if (timer <= nbframes*2)
    {
        float g = pass_action.colors[0].clear_value.g + 0.01f;
        pass_action.colors[0].clear_value.g = (g > 1.0f) ? 0.0f : g;
    }
    else
    {
        float b = pass_action.colors[0].clear_value.b + 0.01f;
        pass_action.colors[0].clear_value.b = (b > 1.0f) ? 0.0f : b;
    }

    if (timer >= nbframes*3)
    {
        timer = 0;
    }
    
    sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());
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
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .width = 800,
        .height = 800,
        .window_title = "Clear (sokol app)",
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}