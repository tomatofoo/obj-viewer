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
            ctx->proj[i] = (point) {0, 0, -1};
            continue;
        }
        ctx->proj[i] = (point) {
            rel.x / rel.z * ctx->flength + ctx->texture->w / 2,
            -rel.y / rel.z * ctx->flength + ctx->texture->h / 2,
            rel.z,
        };
    }

    vec3 forward = (vec3) {0, 0, 1};
    vec3_rot_x_ip(&forward, ctx->rot.x);
    vec3_rot_y_ip(&forward, ctx->rot.y);
    vec3_rot_z_ip(&forward, ctx->rot.z);

    int xmin;
    int xmax;
    int ymin;
    int ymax;
    point points[3];
    size_t n;
    for (size_t i = 0; i < mdl->nfaces; i++) {
        if (vec3_dot(forward, mdl->faces[i].normal) > 0) { continue; }
        xmin = ctx->texture->w;
        xmax = 0;
        ymin = ctx->texture->h;
        ymax = 0;
        for (size_t j = 0; j < 3; j++) {
            points[j] = ctx->proj[mdl->faces[i].vertices[j]];
            if (points[j].x < xmin) {
                xmin = SDL_max(points[j].x, 0);
            }
            if (points[j].x > xmax) {
                xmax = SDL_min(points[j].x, ctx->texture->w);
            }
            if (points[j].y < ymin) {
                ymin = SDL_max(points[j].y, 0);
            }
            if (points[j].y > ymax) {
                ymax = SDL_min(points[j].y, ctx->texture->h);
            }
        }
        
        // Half-space triangle checking
        // https://sw-shader.sourceforge.net/rasterizer.html
        // ^ use wayback machine
        int xdiff[] = {
            points[0].x - points[1].x,
            points[1].x - points[2].x,
            points[2].x - points[0].x
        };
        int ydiff[] = {
            points[0].y - points[1].y,
            points[1].y - points[2].y,
            points[2].y - points[0].y
        };
        // Expressions that get added to/subtracted from
        int yexp[3];
        for (size_t i = 0; i < 3; i++) {
            yexp[i] = (
                xdiff[i] * (ymin - points[i].y)
                - ydiff[i] * (xmin - points[i].x)
                + (ydiff[i] < 0 || (ydiff[i] == 0 && xdiff[i] > 0))
            );
        }
        for (int y = ymin; y < ymax; y++) {
            int xexp[] = {yexp[0], yexp[1], yexp[2]};
            for (int x = xmin; x < xmax; x++) {
                if (xexp[0] > 0 && xexp[1] > 0 && xexp[2] > 0) {
                    n = y * pitch + x * 3;
                    pixels[n + 0] = 255;
                    pixels[n + 1] = 255;
                    pixels[n + 2] = 255;
                }
                for (size_t i = 0; i < 3; i++) { xexp[i] -= ydiff[i]; }
            }
            for (size_t i = 0; i < 3; i++) { yexp[i] += xdiff[i]; }
        }
    }

    SDL_UnlockTexture(ctx->texture);
    SDL_RenderTexture(ctx->renderer, ctx->texture, srcrect, dstrect);

    return true;
}

