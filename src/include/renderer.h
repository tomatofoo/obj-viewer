#ifndef RENDERER_H
#define RENDERER_H


#include "SDL3/SDL.h"

#include "utils.h"


typedef struct uv {
    vec2 v1;
    vec2 v2;
    vec2 v3;
} uv;

typedef struct face {
    // all are indices
    size_t v1;
    size_t v2;
    size_t v3;
    size_t normal;
    size_t uv;
    SDL_Texture *texture;
} face;

typedef struct model {
    size_t nvertices; // amount
    size_t cvertices; // capacity
    vec3 *vertices;
    size_t nnormals;
    size_t cnormals;
    vec3 *normals;
    size_t nuvs;
    size_t cuvs;
    uv *uvs;
    size_t nfaces;
    size_t cfaces;
    face *faces;
} model;

typedef struct context {
    model *model;
    vec3 pos;
    vec3 rot; // rot around x, y, z axes respectively
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} context;


void destroy_model(model *mdl);
context *create_context(const char *path, SDL_Renderer *renderer, int w, int h);
void destroy_context(context *ctx);

void render(context *ctx, const SDL_FRect *srcrect, const SDL_FRect *dstrect);

#endif

