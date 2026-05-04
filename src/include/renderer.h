#ifndef RENDERER_H
#define RENDERER_H


#include "SDL3/SDL.h"

#include "utils.h"


typedef struct material {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    SDL_Surface *texture;
} material;

typedef struct face {
    // all are indices
    size_t vertices[3];
    vec3 centroid;
    int uvs[3]; // int because need -1
    int normals[3];
    vec3 normal; // average of all normals
    int mat; // -1 means none
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
    vec2 *uvs;
    size_t nfaces;
    size_t cfaces;
    face *faces;
    size_t nmats;
    size_t cmats;
    material *mats;
} model;

typedef struct point {
    int x;
    int y;
    double z;
} point;

typedef struct context {
    point *proj; // projected vertices in last frame (also includes depth as z)
    uint32_t *zbuf; // depth buffer
    model *mdl;
    double flength; // focal length
    double brightness;
    vec3 pos;
    vec3 rot; // rot around x, y, z axes respectively
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} context;


void destroy_model(model *mdl);
bool normalize_model(model *mdl);
bool scale_model(model *mdl, double scale);

context *create_context(const char *path, SDL_Renderer *renderer, int w, int h);
void destroy_context(context *ctx);

bool render(context *ctx, const SDL_FRect *srcrect, const SDL_FRect *dstrect);

#endif

