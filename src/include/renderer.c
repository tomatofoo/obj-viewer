#include "SDL3/SDL.h"

#include "utils.h"
#include "renderer.h"


context *create_context(
    const char *path,
    SDL_Renderer *renderer,
    int w, int h
) {
    size_t datasize;
    void *data = SDL_LoadFile(path, &datasize);
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
    }
    SDL_free(data);

    context *ctx = SDL_malloc(sizeof(context));

}

void destroy_context(context *ctx) {
    // more stuff before this
    SDL_free(ctx);
}


void render(context *ctx, const SDL_FRect *srcrect, const SDL_FRect *dstrect) {
}

