#include "SDL3/SDL.h"

#include "parser.h"
#include "renderer.h"
#include "utils.h"


void destroy_model(model *mdl) {
    SDL_free(mdl->vertices);
    SDL_free(mdl->normals);
    SDL_free(mdl->uvs);
    for (size_t i = 0; i < mdl->nfaces; i++) {
        // texture expected to have a refcount
        SDL_free(mdl->faces[i].texture);
    }
    SDL_free(mdl->faces);
    SDL_free(mdl);
}


context *create_context(
    const char *path,
    SDL_Renderer *renderer,
    int w, int h
) {
    context *ctx = SDL_malloc(sizeof(context));
    if (ctx == NULL) {
        SDL_OutOfMemory();
        SDL_SetError(
            "Failed to allocate memory for context: %s", SDL_GetError()
        );
        return NULL;
    }
    // stacking error messages to help tracing
    model *mdl = parse_obj(path);
    if (mdl == NULL) {
        SDL_SetError(
            "Failed to parse OBJ file for context: %s", SDL_GetError()
        );
        return NULL;
    }
    ctx->model = mdl;
    ctx->pos = (vec3) {0, 0, 0};
    ctx->rot = (vec3) {0, 0, 0};
    ctx->renderer = renderer;
    ctx->texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        w, h
    );

    return ctx;
}

void destroy_context(context *ctx) {
    destroy_model(ctx->model);
    SDL_DestroyTexture(ctx->texture);
    SDL_free(ctx);
}


void render(context *ctx, const SDL_FRect *srcrect, const SDL_FRect *dstrect) {
    // more stuff here
    SDL_RenderTexture(ctx->renderer, ctx->texture, srcrect, dstrect);
}

