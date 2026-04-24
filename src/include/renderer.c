#include "SDL3/SDL.h"

#include "utils.h"
#include "renderer.h"


context *create_context(
    const char *path,
    SDL_Renderer *renderer,
    int w, int h
) {
    size_t datasize;
    char *data = SDL_LoadFile(path, &datasize);
    if (data == NULL) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR,
            "Failed to load file: %s\n",
            SDL_GetError()
        );
        return NULL;
    }
    model mdl;
    for (size_t i = 0; i < datasize; i++) {
        if (data[i] == '\r' || data[i] == '\n') {
            SDL_Log("detected newline");
        }
    }
    SDL_free(data);

    context *ctx = SDL_malloc(sizeof(context));
    ctx->model = model;
    ctx->pos = (vec3) {0, 0, 0}
    ctx->rotation = (vec3) {0, 0, 0}
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

