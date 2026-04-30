#define SDL_MAIN_USE_CALLBACKS 1

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include "parser.h"
#include "renderer.h"
#include "utils.h"


// APP VARIABLES
#define APPNAME "OBJ Viewer"
#define APPVERSION "0.1.0"
#define APPIDENTIFIER "com.tomatofu.obj-viewer"
#define WIDTH 640
#define HEIGHT 480
#define FLAGS 0
#define GAMESPEED 1 // game speed
static SDL_Window *window;
static SDL_Renderer *renderer;
static context *ctx;
static Uint64 last; // for timer

static int clicking = -1;
static vec2 vertex1 = (vec2) {WIDTH / 2, 120};
static vec2 vertex2 = (vec2) {WIDTH * 0.75, HEIGHT - 120};
static vec2 vertex3 = (vec2) {WIDTH / 4, HEIGHT - 120};


// APP FUNCTIONS
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata(APPNAME, APPVERSION, APPIDENTIFIER);

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR,
            "Failed to initialize SDL: %s",
            SDL_GetError()
        );
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer(
        APPNAME " " APPVERSION, WIDTH, HEIGHT, FLAGS, &window, &renderer
    )) {
        SDL_LogError(
            SDL_LOG_CATEGORY_VIDEO,
            "Failed to create window and renderer: %s",
            SDL_GetError()
        );
        return SDL_APP_FAILURE;
    }

    last = SDL_GetPerformanceCounter();
    
    ctx = create_context("data/teapot.obj", renderer, WIDTH, HEIGHT);
    if (ctx == NULL) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR,
            "Failed to create context: %s",
            SDL_GetError()
        );
        return SDL_APP_FAILURE;
    }
    SDL_SetTextureScaleMode(ctx->texture, SDL_SCALEMODE_NEAREST);
    model *mdl = ctx->mdl;
    for (size_t i = 0; i < mdl->nvertices; i++) {
        SDL_Log(
            "VERTEX: vec3(%f, %f, %f)",
            mdl->vertices[i].x,
            mdl->vertices[i].y,
            mdl->vertices[i].z
        );
    }
    for (size_t i = 0; i < mdl->nnormals; i++) {
        SDL_Log(
            "NORMAL: vec3(%f, %f, %f)",
            mdl->normals[i].x,
            mdl->normals[i].y,
            mdl->normals[i].z
        );
    }
    for (size_t i = 0; i < mdl->nuvs; i++) {
        SDL_Log(
            "UV: vec2(%f, %f)",
            mdl->uvs[i].x,
            mdl->uvs[i].y
        );
    }
    for (size_t i = 0; i < mdl->nfaces; i++) {
        SDL_Log(
            "FACE: %zu/%d/%d %zu/%d/%d %zu/%d/%d vec3(%f, %f, %f)",
            mdl->faces[i].vertices[0] + 1,
            mdl->faces[i].uvs[0] + 1,
            mdl->faces[i].normals[0] + 1,
            mdl->faces[i].vertices[1] + 1,
            mdl->faces[i].uvs[1] + 1,
            mdl->faces[i].normals[1] + 1,
            mdl->faces[i].vertices[2] + 1,
            mdl->faces[i].uvs[2] + 1,
            mdl->faces[i].normals[2] + 1,
            mdl->faces[i].normal.x,
            mdl->faces[i].normal.y,
            mdl->faces[i].normal.z
        );
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (hypot(event->button.x - vertex1.x, event->button.y - vertex1.y) < 10) {
                clicking = 0;
            }
            else if (hypot(event->button.x - vertex2.x, event->button.y - vertex2.y) < 10) {
                clicking = 1;
            }
            else if (hypot(event->button.x - vertex3.x, event->button.y - vertex3.y) < 10) {
                clicking = 2;
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            clicking = -1;
            break;
        case SDL_EVENT_MOUSE_MOTION:
            if (clicking == 0) {
                vertex1.x += event->motion.xrel;
                vertex1.y += event->motion.yrel;
            }
            if (clicking == 1) {
                vertex2.x += event->motion.xrel;
                vertex2.y += event->motion.yrel;
            }

            if (clicking == 2) {
                vertex3.x += event->motion.xrel;
                vertex3.y += event->motion.yrel;
            }
            break;
    }
    
    return SDL_APP_CONTINUE;
}

void _render(vec2 v1, vec2 v2, vec2 v3) {
    vec2 diff12 = vec2_sub(v1, v2);
    vec2 diff23 = vec2_sub(v2, v3);
    vec2 diff31 = vec2_sub(v3, v1);
    double det = vec2_cross(vec2_sub(v1, v3), diff23);

    int left = SDL_max(SDL_min(SDL_min(v1.x, v2.x), v3.x), 0);
    int right = SDL_min(SDL_max(SDL_max(v1.x, v2.x), v3.x), WIDTH);
    int top = SDL_max(SDL_min(SDL_min(v1.y, v2.y), v3.y), 0);
    int bottom = SDL_min(SDL_max(SDL_max(v1.y, v2.y), v3.y), HEIGHT);
    
    vec2 vec = {0, 0};
    for (int y = top; y < bottom; y++) {
        vec.y = y;
        for (int x = left; x < right; x++) {
            vec.x = x;
            vec2 diff03 = vec2_sub(vec, v3);
            double l1 = vec2_cross(diff03, diff23) / det;
            double l2 = vec2_cross(diff03, diff31) / det;
            double l3 = vec2_cross(vec2_sub(vec, v1), diff12) / det;
            
            if (l1 < 0 || l2 < 0 || l3 < 0) {
                continue;
            }
            if (l1 > 1 || l2 > 1 || l3 > 1) {
                continue;
            }
            SDL_SetRenderDrawColor(
                renderer,
                l1 * 255, l2 * 255, l3 * 255,
                SDL_ALPHA_OPAQUE
            );
            SDL_RenderPoint(renderer, x, y);
        }
    }

    SDL_SetRenderDrawColor(renderer, WHITE, SDL_ALPHA_OPAQUE);
    SDL_RenderRect(renderer, &(SDL_FRect) {v1.x - 5, v1.y - 5, 10, 10});
    SDL_RenderRect(renderer, &(SDL_FRect) {v2.x - 5, v2.y - 5, 10, 10});
    SDL_RenderRect(renderer, &(SDL_FRect) {v3.x - 5, v3.y - 5, 10, 10});
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    // DELTA TIME
    double now = SDL_GetPerformanceCounter();
    double freq = SDL_GetPerformanceFrequency();
    double timer = now / freq;
    double dt = (now - last) / freq * GAMESPEED;
    last = now;

    // UPDATE
    const bool *keys = SDL_GetKeyboardState(NULL);
    ctx->rot.x += (keys[SDL_SCANCODE_DOWN] - keys[SDL_SCANCODE_UP]) * dt;
    ctx->rot.y += (keys[SDL_SCANCODE_RIGHT] - keys[SDL_SCANCODE_LEFT]) * dt;
    vec3 mvt = (vec3) {
        keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A],
        keys[SDL_SCANCODE_SPACE] - keys[SDL_SCANCODE_LSHIFT],
        keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S]
    };
    vec3_rot_y_ip(&mvt, ctx->rot.y);
    ctx->pos.x += mvt.x * dt;
    ctx->pos.y += mvt.y * dt;
    ctx->pos.z += mvt.z * dt;

    // RENDER
    // vertex2.x = SDL_sin(timer) * 120 + WIDTH / 2;
    // vertex3.x = SDL_cos(timer) * 120 + WIDTH / 2;
    // _render(vertex1, vertex2, vertex3);
    if (!render(ctx, NULL, NULL)) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR,
            "Failed to render context: %s",
            SDL_GetError()
        );
        return SDL_APP_FAILURE;
    }
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    destroy_context(ctx);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

