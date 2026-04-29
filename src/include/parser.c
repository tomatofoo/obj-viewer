#include "SDL3/SDL.h"

#include "parser.h"
#include "renderer.h"


#define ARR_SIZE 8 // begninning array size for model


typedef enum etype { // Element Types
    NONE,
    VERTEX,
    NORMAL,
    TEXTURE,
    FACE,
} etype;


bool streq_space(const char *str1, const char *str2) {
    while (*str1 == *str2) {
        str1++;
        str2++;
        if ((SDL_isspace(*str1) || !*str1)
            && (SDL_isspace(*str2) || !*str2)) {
            return true;
        }
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
    mdl->vertices = SDL_malloc(sizeof(vec3));
    if (mdl->vertices == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }
    mdl->normals = SDL_malloc(sizeof(vec3));
    if (mdl->normals == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }
    mdl->textures = SDL_malloc(sizeof(texture));
    if (mdl->textures == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }
    mdl->faces = SDL_malloc(sizeof(face));
    if (mdl->faces == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }

    // ACTUAL PARSING
    etype elem = NONE;
    bool begin; // finished parsing element type; now will parse elem
    bool cont = false; // continue (e.g. comment, group, etc.)
    bool start = true; // start of new item (inside element)
    bool end; // if is last char of current item
    bool neg; // if number value (see below) is negative
    double value; // a number value (vertices, normals, etc.)
    int power = -1; // power for decimal numbers (might need to be size_t)
    size_t n; // index for arrays
    for (size_t i = 0; i < datasize; i++) {
        if (data[i] == '\r' || data[i] == '\n') {
            elem = NONE;
            cont = false;
            start = true;
            continue;
        }
        if (SDL_isspace(data[i]) || cont) {
            start = true;
            begin = true;
            continue;
        }
        if (elem == NONE) {
            begin = false; // ^ waits until next whitespace to be true
            if (data[i] == '#') { cont = true; }
            else if (streq_space(data + i, "g")) { cont = true; }
            else if (streq_space(data + i, "o")) { cont = true; }
            else if (streq_space(data + i, "v")) { elem = VERTEX; }
            else if (streq_space(data + i, "vn")) { elem = NORMAL; }
            else if (streq_space(data + i, "vt")) { elem = TEXTURE; }
            else if (streq_space(data + i, "f")) { elem = FACE; }
            continue;
        }
        if (!begin) { continue; } // will start parsing after beginning
        end = ( // check if is end
            data[i + 1] == '\r' || data[i + 1] == '\n'
            || SDL_isspace(data[i + 1]) || cont
        );
        // Number Parsing
        if (elem == VERTEX || elem == NORMAL || elem == TEXTURE) {
            if (start) {
                value = 0;
                neg = false;
                power = -1;
                start = false;
                if (data[i] == '-') {
                    neg = true;
                    continue;
                }
            }
            if (data[i] == '.') {
                power = 0;
                continue;
            }
            if (power > -1) { power++; }
            value = value * 10 + (data[i] - '0');
            if (end) {
                value /= SDL_pow(10, power);
                if (neg) { value = -value; }
            }
        }
        if (elem == VERTEX) {
            if (end) {
            }
        }
        else if (elem == NORMAL) {
        }
        else if (elem == TEXTURE) {
        }
        else if (elem == FACE) {
        }
    }
    // FREE UP DATA AFTER PARSING
    SDL_free(data);

    return mdl;
}

