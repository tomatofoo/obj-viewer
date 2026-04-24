#include "SDL3/SDL.h"

#include "renderer.h"


bool parse_obj(const char *path, model *mdl) {
    size_t datasize;
    char *data = SDL_LoadFile(path, &datasize);
    if (data == NULL) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR,
            "Failed to load file: %s\n",
            SDL_GetError()
        );
        return false;
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

    return true;
}

