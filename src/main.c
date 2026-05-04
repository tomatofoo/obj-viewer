#define SDL_MAIN_USE_CALLBACKS 1

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "SDL3_image/SDL_image.h"
#include "SDL3_ttf/SDL_ttf.h"

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
#define FONTSIZE 14

#define SCANCODE_FORWARD SDL_SCANCODE_W
#define SCANCODE_BACKWARD SDL_SCANCODE_S
#define SCANCODE_LEFT SDL_SCANCODE_A
#define SCANCODE_RIGHT SDL_SCANCODE_D
#define SCANCODE_UP SDL_SCANCODE_SPACE
#define SCANCODE_DOWN SDL_SCANCODE_LSHIFT
#define SCANCODE_LOOK_LEFT SDL_SCANCODE_LEFT
#define SCANCODE_LOOK_RIGHT SDL_SCANCODE_RIGHT
#define SCANCODE_LOOK_UP SDL_SCANCODE_UP
#define SCANCODE_LOOK_DOWN SDL_SCANCODE_DOWN
#define SCANCODE_SCREENSHOT SDL_SCANCODE_F2

static SDL_Window *window;
static SDL_Renderer *renderer;
static context *ctx = NULL;
static TTF_Font *font;
static Uint64 last; // for timer

static bool save_scrshot_failed; // separate because of threads
static bool drop_file_failed;

static const SDL_DialogFileFilter filters_scrshot[] = {
    { "PNG images",  "png" },
    { "JPEG images", "jpg;jpeg;jpe" },
    { "All images",  "png;jpg;jpeg;jpe" },
    { "All files",   "*" }
};
static SDL_Texture *textures[2];
static SDL_FRect rects[2]; // for textures


void SDLCALL save_scrshot(void *userdata) {
    const char * const *filelist = userdata;

    if (ctx == NULL) { return; }
    if (filelist == NULL) {
        SDL_SetError("File list cannot be NULL.");
        save_scrshot_failed = true;
        return;
    }
    if (*filelist == NULL) {
        SDL_SetError("A file cannot be NULL.");
        save_scrshot_failed = true;
        return;
    }
    
    const char *filename;
    const char *ext;
    // RenderReadPixels has to be before a RenderPresent
    SDL_RenderTexture(renderer, ctx->texture, NULL, NULL);
    SDL_Surface *surf = SDL_RenderReadPixels(renderer, NULL);
    if (surf == NULL) {
        SDL_SetError(
            "Failed to create surface from renderer: %s",
            SDL_GetError()
        );
        save_scrshot_failed = true;
        return;
    }

    while (*filelist) {
        filename = *filelist;
        ext = filename_ext(filename);
        if (ext != NULL
            && (SDL_strcmp(ext, "jpg") == 0 || SDL_strcmp(ext, "JPG") == 0
                || SDL_strcmp(ext, "jpeg") == 0 || SDL_strcmp(ext, "JPEG") == 0
                || SDL_strcmp(ext, "jpe") == 0 || SDL_strcmp(ext, "JPE") == 0)
            && !IMG_SaveJPG(surf, filename, 100)) {
            SDL_DestroySurface(surf);
            SDL_SetError(
                "Failed to save to surface to JPG: %s",
                SDL_GetError()
            );
            save_scrshot_failed = true;
            return;
        }
        else if (!IMG_SavePNG(surf, filename)) {
            SDL_DestroySurface(surf);
            SDL_SetError(
                "Failed to save to surface to PNG: %s",
                SDL_GetError()
            );
            save_scrshot_failed = true;
            return;
        }
        filelist++;
    }

    SDL_DestroySurface(surf);
}

void SDLCALL save_scrshot_thread(
    void *userdata, const char * const *filelist, int filter
) {
    SDL_RunOnMainThread(*save_scrshot, (void *) filelist, true);
    if (save_scrshot_failed) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR,
            "Failed to save screenshot: %s",
            SDL_GetError()
        );
        return;
    }
}

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

bool render_text(const char *text, SDL_Texture **texture, SDL_FRect *rect) {
    SDL_Surface *surf = TTF_RenderText_Shaded(
        font, text, 0, (SDL_Color) {WHITE}, (SDL_Color) {BLACK}
    );
    if (surf == NULL) {
        SDL_SetError("Failed to render text: %s", SDL_GetError());
        return false;
    }
    *texture = SDL_CreateTextureFromSurface(renderer, surf);
    if (*texture == NULL) {
        SDL_DestroySurface(surf);
        SDL_SetError(
            "Failed to create texture from surface: %s",
            SDL_GetError()
        );
        return false;
    }
    *rect = (SDL_FRect) {
        (WIDTH - (*texture)->w) / 2,
        (HEIGHT - (*texture)->h) / 2,
        (*texture)->w,
        (*texture)->h
    };
    SDL_DestroySurface(surf);

    return true;
}

void SDLCALL drop_file(void *userdata) {
    SDL_Event *event = userdata;

    // Add loading text
    if (!SDL_RenderClear(renderer)) {
        SDL_SetError("Failed to clear renderer: %s", SDL_GetError());
        drop_file_failed = true;
        return;
    }
    if (!SDL_RenderTexture(renderer, textures[1], NULL, rects + 1)) {
        SDL_SetError("Failed to render texture: %s", SDL_GetError());
        drop_file_failed = true;
        return;
    }
    if (!SDL_RenderPresent(renderer)) {
        SDL_SetError("Failed to present renderer: %s", SDL_GetError());
        drop_file_failed = true;
        return;
    }
    // Load file
    if (!load_file(event->drop.data)) {
        SDL_SetError("Failed to load file: %s", SDL_GetError());
        drop_file_failed = true;
        return;
    }
}

bool drop_file_thread(SDL_Event *event) {
    SDL_RunOnMainThread(*drop_file, (void *) event, true);
    return !drop_file_failed;
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
    if (!TTF_Init()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR,
            "Failed to initialize SDL_ttf: %s",
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

    if (!SDL_SetRenderDrawColor(renderer, BLACK, SDL_ALPHA_OPAQUE)) {
        SDL_LogError(
            SDL_LOG_CATEGORY_RENDER,
            "Failed to set render draw color: %s",
            SDL_GetError()
        );
        return SDL_APP_FAILURE;
    }

    font = TTF_OpenFont("data/fonts/MonaSans-Regular.ttf", FONTSIZE);
    if (font == NULL) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR,
            "Failed to load font: %s",
            SDL_GetError()
        );
        return SDL_APP_FAILURE;
    }
    if (!render_text(
        "Drag a file to this window to load it.",
        textures + 0,
        rects + 0
    )) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR,
            "Failed to render text: %s",
            SDL_GetError()
        );
        return SDL_APP_FAILURE;
    }
    if (!render_text("Loading...", textures + 1, rects + 1)) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR,
            "Failed to render text: %s",
            SDL_GetError()
        );
        return SDL_APP_FAILURE;
    }

    last = SDL_GetPerformanceCounter();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
            break;
        case SDL_EVENT_DROP_FILE:
            if (!drop_file_thread(event)) {
                SDL_LogError(
                    SDL_LOG_CATEGORY_ERROR,
                    "Failed to drop file: %s",
                    SDL_GetError()
                );
                return SDL_APP_FAILURE;
            }
            break;
        case SDL_EVENT_KEY_DOWN:
            // If this fails it won't close app
            if (event->key.scancode == SCANCODE_SCREENSHOT) {
                SDL_ShowSaveFileDialog(
                    *save_scrshot_thread,
                    NULL,
                    window,
                    filters_scrshot,
                    arr_sizeof(filters_scrshot),
                    NULL
                );
            }
            break;
        default:
            break;
    }
    
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    // DELTA TIME
    double now = SDL_GetPerformanceCounter();
    double freq = SDL_GetPerformanceFrequency();
    double timer = now / freq;
    double dt = (now - last) / freq * GAMESPEED;
    last = now;
    
    // Recommended to clear even if overwriting every px
    if (!SDL_RenderClear(renderer)) {
        SDL_LogError(
            SDL_LOG_CATEGORY_RENDER,
            "Failed to clear renderer: %s",
            SDL_GetError()
        );
        return SDL_APP_FAILURE;
    }
    if (ctx == NULL) {
        if (!SDL_RenderTexture(renderer, textures[0], NULL, rects + 0)) {
            SDL_LogError(
                SDL_LOG_CATEGORY_RENDER,
                "Failed to render texture: %s",
                SDL_GetError()
            );
            return SDL_APP_FAILURE;
        }
    }
    else {
        // UPDATE
        const bool *keys = SDL_GetKeyboardState(NULL);
        
        double rspeed = 1.2;
        double mspeed = 0.9;
        vec3 rot = (vec3) {
            keys[SCANCODE_LOOK_DOWN] - keys[SCANCODE_LOOK_UP],
            keys[SCANCODE_LOOK_RIGHT] - keys[SCANCODE_LOOK_LEFT],
            0
        };
        ctx->rot.x += rot.x * rspeed * dt;
        ctx->rot.y += rot.y * rspeed * dt;
        ctx->rot.z += rot.z * rspeed * dt;
        vec3 mvt = (vec3) {
            keys[SCANCODE_RIGHT] - keys[SCANCODE_LEFT],
            keys[SCANCODE_UP] - keys[SCANCODE_DOWN],
            keys[SCANCODE_FORWARD] - keys[SCANCODE_BACKWARD]
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
    }

    if (!SDL_RenderPresent(renderer)) {
        SDL_LogError(
            SDL_LOG_CATEGORY_RENDER,
            "Failed to present renderer: %s",
            SDL_GetError()
        );
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    for (size_t i = 0; i < arr_sizeof(textures); i++) {
        SDL_DestroyTexture(textures[i]);
    }
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    destroy_context(ctx);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
}

