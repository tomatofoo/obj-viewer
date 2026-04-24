#include "SDL3/SDL.h"

#include "parser.h"
#include "renderer.h"
#include "utils.h"


context *create_context(
    const char *path,
    SDL_Renderer *renderer,
    int w, int h
) {

    context *ctx = SDL_malloc(sizeof(context));
    if (ctx == NULL) {
        SDL_OutOfMemory();
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR,
            "Failed to allocate memory: %s",
            SDL_GetError()
        );
        return NULL;
    }
    model mdl;
    // error logging is handled at the farthest depth
    if (!parse_obj(path, &mdl))  { return NULL; }
    ctx->model = mdl;
    ctx->pos = (vec3) {0, 0, 0};
    ctx->rot = (vec3) {0, 0, 0};
    ctx->texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        w, h
    );

    return ctx;
}

void destroy_context(context *ctx) {
    // more stuff here
    SDL_DestroyTexture(ctx->texture);
    SDL_free(ctx);
}


void render(context *ctx, const SDL_FRect *srcrect, const SDL_FRect *dstrect) {
}

