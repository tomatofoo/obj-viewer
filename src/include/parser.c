#include "SDL3/SDL.h"

#include "parser.h"
#include "renderer.h"


typedef enum etype { // Element Types
    NONE,
    VERTEX,
    NORMAL,
    TEXTURE,
    FACE,
} etype;


bool streq_space(const char *str1, const char *str2) {
    while (str1[i] == str2[i]) {
        if (SDL_isspace(*str1) || !*str1) { return true; }
        str1++;
        str2++;
    }
    return false;
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

    // ACTUAL PARSING
    etype elem = NONE;
    bool cont = false; // continue (e.g. comment, group, etc.)
    bool start = true; // start of new item (inside element)
    for (size_t i = 0; i < datasize; i++) {
        if (SDL_isspace(data[i]) || cont) {
            start = true;
            continue;
        }
        if (data[i] == '\r' || data[i] == '\n') {
            elem = NONE;
            cont = false;
            start = true;
            continue;
        }
        if (elem == NONE) {
            if (data[i] == '#') { cont = true; }
            else if (streq_space(&data[i], "g")) { cont = true; }
            else if (streq_space(&data[i], "o")) { cont = true; }
            else if (streq_space(&data[i], "v")) { elem = VERTEX; }
            else if (streq_space(&data[i], "vn")) { elem = NORMAL; }
            else if (streq_space(&data[i], "vt")) { elem = TEXTURE; }
            else if (streq_space(&data[i], "f")) { elem = FACE; }
            continue;
        }
        if (elem == VERTEX) {
            if (start) {
            }
        }
        start = false;
    }
    // FREE UP DATA AFTER PARSING
    SDL_free(data);

    return mdl;
}

