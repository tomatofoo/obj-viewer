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
static context *ctx = NULL;
static Uint64 last; // for timer

static int clicking = -1;
static vec2 vertex1 = (vec2) {WIDTH / 2, 120};
static vec2 vertex2 = (vec2) {WIDTH * 0.75, HEIGHT - 120};
static vec2 vertex3 = (vec2) {WIDTH / 4, HEIGHT - 120};


bool load_file(const char *path) {
    context *new = create_context(path, renderer, WIDTH, HEIGHT);
    if (new == NULL) {
        SDL_SetError(
            "Failed to create context: %s",
            SDL_GetError()
        );
        return false;
    }
    if (!SDL_SetTextureScaleMode(new->texture, SDL_SCALEMODE_NEAREST)) {
        destroy_context(new);
        SDL_SetError(
            "Failed to set texture scale mode: %s",
            SDL_GetError()
        );
        return false;
    }
    if (!normalize_model(new->mdl)) {
        destroy_context(new);
        SDL_SetError(
            "Failed to normalize model: %s",
            SDL_GetError()
        );
        return false;
    }

    if (ctx != NULL) { destroy_context(ctx); }
    ctx = new;

    return true;
}


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
    
    if (!load_file("data/crate.obj")) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR,
            "Failed to load file: %s",
            SDL_GetError()
        );
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
            break;
        case SDL_EVENT_DROP_FILE:
            load_file(event->drop.data);
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
    
    double rspeed = 1.2;
    double mspeed = 0.9;
    vec3 rot = (vec3) {
        keys[SDL_SCANCODE_DOWN] - keys[SDL_SCANCODE_UP],
        keys[SDL_SCANCODE_RIGHT] - keys[SDL_SCANCODE_LEFT],
        0
    };
    ctx->rot.x += rot.x * rspeed * dt;
    ctx->rot.y += rot.y * rspeed * dt;
    ctx->rot.z += rot.z * rspeed * dt;
    vec3 mvt = (vec3) {
        keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A],
        keys[SDL_SCANCODE_SPACE] - keys[SDL_SCANCODE_LSHIFT],
        keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S]
    };
    vec3_rot_y_ip(&mvt, ctx->rot.y);
    ctx->pos.x += mvt.x * mspeed * dt;
    ctx->pos.y += mvt.y * mspeed * dt;
    ctx->pos.z += mvt.z * mspeed * dt;

    // RENDER
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

