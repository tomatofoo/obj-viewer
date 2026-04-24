#include "SDL3/SDL.h"

#include "renderer.h"


model *parse_obj(const char *path) {
    size_t datasize;
    char *data = SDL_LoadFile(path, &datasize);
    if (data == NULL) {
        SDL_SetError("Failed to load OBJ file: %s", SDL_GetError());
        return NULL;
    }
    model *mdl = SDL_malloc(sizeof(mdl));
    if (mdl == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }
    char type;
    bool cont = false; // continue (e.g. comment, g)
    for (size_t i = 0; i < datasize; i++) {
        // Linebreaks
        if (data[i] == '\r' || data[i] == '\n') {
            type = '\0';
        }
    }
    SDL_free(data);

    return mdl;
}

