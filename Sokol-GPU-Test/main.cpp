//------------------------------------------------------------------------------
//  triangle-sapp.c
//  Simple 2D rendering from vertex buffer.
//------------------------------------------------------------------------------
#define SOKOL_IMPL
#define SOKOL_D3D11
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#include "triangle-sapp.glsl.h"
#include "stdio.h"
#include <vector>

// application state
static struct
{
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
} state;
enum Pivot
{
    Center,
    TopLeft,
};

struct Color
{
    float r, g, b, a;
    Color(float rVal, float gVal, float bVal, float aVal) : r(rVal), g(gVal), b(bVal), a(aVal) {}
};
const Color BLACK(0, 0, 0, 1);
const Color WHITE(1, 1, 1, 1);
const Color RED(1, 0, 0, 1);
const Color GREEN(0, 1, 0, 1);
const Color BLUE(0, 0, 1, 1);
const Color CYAN(0, 1, 1, 1);
const Color YELLOW(1, 1, 0, 1);
const Color MAGENTA(1, 0, 1, 1);

struct Vertex
{
    float x, y, z;
    Color color;

    Vertex(float xVal, float yVal, float zVal, Color cVal) : x(xVal), y(yVal), z(zVal), color(cVal) {}
};

int frameCount = 0;

float vertexBuffer[1000];
int vertexBufferUsed = 0;

uint32_t indexBuffer[1000];
int indexBufferUsed = 0;

void AppendVertex(Vertex v)
{
    int winWidth = sapp_width();
    int winHeight = sapp_height();

    vertexBuffer[vertexBufferUsed++] = v.x;
    vertexBuffer[vertexBufferUsed++] = v.y;
    vertexBuffer[vertexBufferUsed++] = v.z;
    vertexBuffer[vertexBufferUsed++] = v.color.r;
    vertexBuffer[vertexBufferUsed++] = v.color.g;
    vertexBuffer[vertexBufferUsed++] = v.color.b;
    vertexBuffer[vertexBufferUsed++] = v.color.a;
}
void DrawRect(float x, float y, float w, float h, Color c, Pivot p = Center)
{
    float z = 0.5f;
    int startIndex = vertexBufferUsed / (3 + 4);
    if (p == Center)
    {
        x -= w / 2;
        y -= h / 2;
    }

    AppendVertex(Vertex(x, y, z, c));
    AppendVertex(Vertex(x + w, y, z, c));
    AppendVertex(Vertex(x + w, y + h, z, c));
    AppendVertex(Vertex(x, y + h, z, c));

    indexBuffer[indexBufferUsed++] = startIndex + 0;
    indexBuffer[indexBufferUsed++] = startIndex + 1;
    indexBuffer[indexBufferUsed++] = startIndex + 2;

    indexBuffer[indexBufferUsed++] = startIndex + 0;
    indexBuffer[indexBufferUsed++] = startIndex + 2;
    indexBuffer[indexBufferUsed++] = startIndex + 3;
}

void DrawCircle(float x, float y, float r, Color c, int sides = 50, bool pacman = false)
{
    const int numSegments = sides; // Adjust the number of segments for smoother circles

    float z = 0.5f;
    int startIndex = vertexBufferUsed / (3 + 4);

    // Center vertex
    AppendVertex(Vertex(x, y, z, c));

    // Calculate the angle between each segment
    float angleStep = (pacman ? 1.5f : 2.0f) * 3.14159f / numSegments;
     float bAngle = sides % 2 == 0 ? 0.0f : 3.14159f / (numSegments * 2.0f); // Make polygons symmetric on the screen
    float currentAngle = pacman ? 0.7f : bAngle;

    for (int i = 0; i <= numSegments; i++)
    {
        // Calculate the position of the current vertex
        float vertexX = x + r * cos(currentAngle);
        float vertexY = y + r * sin(currentAngle);

        // Append the vertex to the buffer
        AppendVertex(Vertex(vertexX, vertexY, z, c));

        // Add the indices to form triangles using the center vertex
        if (i > 0)
        {
            indexBuffer[indexBufferUsed++] = startIndex;
            indexBuffer[indexBufferUsed++] = startIndex + i;
            indexBuffer[indexBufferUsed++] = startIndex + i + 1;
        }

        // Update the current angle
        currentAngle += angleStep;
    }
}
void DrawPolygon(float x, float y, float size, Color c, int sides)
{
    DrawCircle(x, y, size / 2, c, sides);
}
static void init(void)
{
    sg_desc desc = {
        .context = sapp_sgcontext(),
        .logger.func = slog_func,
    };
    sg_setup(desc);

    // a vertex buffer with 3 vertices

    // positions x,y       // colors r,g,b,a
    DrawRect(-0.5f, -0.5f, 0.2f, 0.2f, CYAN);
    DrawCircle(0, 0, 0.3f, YELLOW);
    DrawPolygon(-0.5f, 0.5f, 0.4f, MAGENTA, 9);
    DrawPolygon(0.5f, 0.5f, 0.6f, GREEN, 11);

    state.bind.vertex_buffers[0] = sg_make_buffer((sg_buffer_desc){
        //.size = sizeof(vertexBuffer),
        .data = sg_range{.ptr = vertexBuffer, .size = vertexBufferUsed * sizeof(*vertexBuffer)},
        .label = "triangle-vertices"});

    state.bind.index_buffer = sg_make_buffer((sg_buffer_desc){
        // .size = sizeof(indexBuffer),
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = sg_range{.ptr = indexBuffer, .size = indexBufferUsed * sizeof(*indexBuffer)},
        .label = "triangle-indices"});

    // create shader from code-generated sg_shader_desc
    sg_shader shd = sg_make_shader(triangle_shader_desc(sg_query_backend()));

    // create a pipeline object (default render states are fine for triangle)
    sg_pipeline_desc pd{
        .index_type = SG_INDEXTYPE_UINT32,
        .shader = shd,
        // if the vertex layout doesn't have gaps, don't need to provide strides and offsets
        .layout = {
            .attrs = {
                [ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4}},
        .label = "triangle-pipeline"};

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