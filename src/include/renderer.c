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
    ctx->mdl = parse_obj(path);
    if (ctx->mdl == NULL) {
        SDL_SetError(
            "Failed to parse OBJ file for context: %s", SDL_GetError()
        );
        return NULL;
    }
    ctx->proj = SDL_malloc(sizeof(vec3) * ctx->mdl->cvertices);
    if (ctx->proj == NULL) {
        SDL_SetError(
            "Failed allocate memory for projected points: %s", SDL_GetError()
        );
        return NULL;
    }
    ctx->flength = w / 2;
    ctx->pos = (vec3) {0, 0, 0};
    ctx->rot = (vec3) {0, 0, 0};
    ctx->renderer = renderer;
    ctx->texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        w, h
    );
    if (ctx->texture == NULL) { 
        SDL_SetError("Failed to create texture: %s", SDL_GetError());
        return NULL;
    }

    return ctx;
}

void destroy_context(context *ctx) {
    destroy_model(ctx->mdl);
    SDL_DestroyTexture(ctx->texture);
    SDL_free(ctx);
}


bool render(context *ctx, const SDL_FRect *srcrect, const SDL_FRect *dstrect) {
    uint8_t *pixels;
    int pitch;
    if (!SDL_LockTexture(ctx->texture, NULL, (void **) &pixels, &pitch)) {
        SDL_SetError("Failed to lock texture for writing: %s", SDL_GetError());
        return false;
    }

    // Clear Texture
    SDL_memset(pixels, 0, ctx->texture->h * pitch);
    
    // Actual Rendering
    model *mdl = ctx->mdl;
    vec3 rel;
    for (size_t i = 0; i < mdl->nvertices; i++) {
        rel = vec3_sub(mdl->vertices[i], ctx->pos);
        vec3_rot_y_ip(&rel, -ctx->rot.y);
        vec3_rot_x_ip(&rel, -ctx->rot.x);
        vec3_rot_z_ip(&rel, -ctx->rot.z);
        if (rel.z <= 0) {
            ctx->proj[i] = (pixel) {-1, -1, -1};
            continue;
        }
        ctx->proj[i] = (pixel) {
            rel.x / rel.z * ctx->flength + ctx->texture->w / 2,
            -rel.y / rel.z * ctx->flength + ctx->texture->h / 2,
            rel.z,
        };
    }
    
    size_t j;
    for (size_t i = 0; i < mdl->nvertices; i++) {
        if (inrange(ctx->proj[i].x, 0, ctx->texture->w, true, false)
            && inrange(ctx->proj[i].y, 0, ctx->texture->h, true, false)) {
            j = ctx->proj[i].y * pitch + ctx->proj[i].x * 3;
            pixels[j + 0] = 255;
            pixels[j + 1] = 255;
            pixels[j + 2] = 255;
        }
    }

    SDL_UnlockTexture(ctx->texture);
    SDL_RenderTexture(ctx->renderer, ctx->texture, srcrect, dstrect);

    return true;
}

