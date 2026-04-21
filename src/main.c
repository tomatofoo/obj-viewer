#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define SDL_MAIN_USE_CALLBACKS 1

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include "utils.h"


// COLORS
#define BLACK 0, 0, 0
#define WHITE 255, 255, 255

// APP VARIABLES
#define APPNAME "OBJ Viewer"
#define APPVERSION "0.1.0"
#define APPIDENTIFIER "com.tomatofu.obj-viewer"
#define WIDTH 640
#define HEIGHT 480
#define FLAGS SDL_WINDOW_RESIZABLE
static SDL_Window *window;
static SDL_Renderer *renderer;
static Uint64 last; // for timer


// APP FUNCTIONS
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata(APPNAME, APPVERSION, APPIDENTIFIER);
    
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer(APPNAME " " APPVERSION, WIDTH, HEIGHT, FLAGS, &window, &renderer)) {
        SDL_Log("Failed to create window and renderer: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    last = SDL_GetPerformanceCounter();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

void render(vec2 v1, vec2 v2, vec2 v3) {
    vec2 diff23 = vec2_sub(v2, v3);
    double det = vec2_cross(vec2_sub(v1, v3), diff23);
    
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            vec2 vec = (vec2) {x, y};
            vec2 diff03 = vec2_sub(vec, v3);
            double l1 = vec2_cross(diff03, diff23) / det;
            double l2 = vec2_cross(diff03, vec2_sub(v3, v1)) / det;
            double l3 = vec2_cross(vec2_sub(vec, v1), vec2_sub(v1, v2)) / det;

            SDL_SetRenderDrawColor(renderer, l1 * 255, l2 * 255, l3 * 255, SDL_ALPHA_OPAQUE);
            SDL_RenderPoint(renderer, x, y);
        }
    }
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    // DELTA TIME
    double dt = (SDL_GetPerformanceCounter() - last) / (double) SDL_GetPerformanceFrequency();
    last = SDL_GetPerformanceCounter(); 
    
    // RENDER
    SDL_SetRenderDrawColor(renderer, BLACK, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    
    render((vec2) {WIDTH / 2, 0}, (vec2) {WIDTH * 0.75, HEIGHT}, (vec2) {WIDTH / 4, HEIGHT});

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
}

