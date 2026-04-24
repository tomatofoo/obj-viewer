#include "SDL3/SDL.h"

#include "renderer.h"


char *read_until_whitespace(size_t i, ) {
}


model *parse_obj(const char *path) {
    // PREAMBLE
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
    // PARSING
    char type;
    bool cont = false; // continue (e.g. comment, group, etc.)
    for (size_t i = 0; i < datasize; i++) {
        // Linebreaks
        if (data[i] == '\r' || data[i] == '\n') {
            type = '\0';
        }
    }
    // FREE UP DATA AFTER PARSING
    SDL_free(data);

    return mdl;
}

