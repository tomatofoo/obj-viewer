#include "SDL3/SDL.h"

#include "parser.h"
#include "renderer.h"


#define ARR_SIZE 8 // begninning array size for model
#define ARR_FACTOR 2 // factor when resizing


typedef enum etype { // Element Types
    NONE,
    VERTEX,
    NORMAL,
    UV,
    FACE,
} etype;


bool isnewline(const char c) {
    return c == '\r' || c == '\n';
}

bool isempty(const char c) {
    return !c || SDL_isspace(c) || isnewline(c);
}

bool streq_space(const char *str1, const char *str2) {
    while (*str1 == *str2) {
        str1++;
        str2++;
        if (isempty(*str1) && isempty(*str2)) { return true; }
    }
    return false;
}


model *parse_obj(const char *path) {
    size_t datasize;
    char *data = SDL_LoadFile(path, &datasize);
    if (data == NULL) {
        SDL_SetError("Failed to load OBJ file: %s", SDL_GetError());
        return NULL;
    }
    model *mdl = SDL_malloc(sizeof(model));
    if (mdl == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }
    // Using malloc instead of calloc because we have nvertices
    // malloc is adequate and faster
    mdl->nvertices = 0;
    mdl->vertices = SDL_malloc(sizeof(vec3) * ARR_SIZE);
    if (mdl->vertices == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }
    mdl->cvertices = ARR_SIZE;
    mdl->nnormals = 0;
    mdl->normals = SDL_malloc(sizeof(vec3) * ARR_SIZE);
    if (mdl->normals == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }
    mdl->cnormals = ARR_SIZE;
    mdl->nuvs = 0;
    mdl->uvs = SDL_malloc(sizeof(uv) * ARR_SIZE);
    if (mdl->uvs == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }
    mdl->cuvs = ARR_SIZE;
    mdl->nfaces = 0;
    mdl->faces = SDL_malloc(sizeof(face) * ARR_SIZE);
    if (mdl->faces == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }
    mdl->cfaces = ARR_SIZE;

    // PER ELEMENT
    etype elem = NONE;
    bool cont = false; // continue (e.g. comment, group, etc.)
    bool begin; // finished parsing element type; now will parse elem
    size_t n; // index for arrays (resets at start)
    // PER ITEM
    bool start = true; // start of new item (inside element)
    bool end; // if is last char of current item
    bool neg; // if number value (see below) is negative
    double value; // a number value (vertices, normals, etc.)
    int power = -1; // power for decimal numbers (might need to be size_t)
    for (size_t i = 0; i < datasize; i++) {
        if (isnewline(data[i])) {
            cont = false;
            begin = false; // waits until next whitespace to be true
            n = 0;
            elem = NONE;
            start = true;
            continue;
        }
        if (SDL_isspace(data[i]) || cont) {
            start = true;
            begin = true;
            continue;
        }
        if (elem == NONE) {
            if (data[i] == '#') { cont = true; }
            else if (streq_space(data + i, "g")) { cont = true; }
            else if (streq_space(data + i, "o")) { cont = true; }
            else if (streq_space(data + i, "v")) { elem = VERTEX; }
            else if (streq_space(data + i, "vn")) { elem = NORMAL; }
            else if (streq_space(data + i, "vt")) { elem = UV; }
            else if (streq_space(data + i, "f")) { elem = FACE; }
            continue;
        }
        if (!begin) { continue; } // will start parsing after beginning
        end = isempty(data[i + 1]) || cont; // check if is end
        // Number Parsing
        if (elem == VERTEX || elem == NORMAL || elem == UV) {
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
        if (elem == VERTEX && end) { // don't need to initialize item in array
            if (n == 0) { mdl->vertices[mdl->nvertices].x = value; }
            else if (n == 1) { mdl->vertices[mdl->nvertices].y = value; }
            else if (n == 2) {
                mdl->vertices[mdl->nvertices].z = value;
                mdl->nvertices++;
                if (mdl->nvertices >= mdl->cvertices) {
                    mdl->vertices = SDL_realloc(
                        mdl->vertices,
                        sizeof(vec3) * mdl->cvertices * ARR_FACTOR
                    );
                    if (mdl->vertices == NULL) {
                        SDL_OutOfMemory();
                        return NULL;
                    }
                    mdl->cvertices *= ARR_FACTOR;
                }
            }
            // w is scaling divisor
            else { vec3_div_ip(mdl->vertices + mdl->nvertices - 1, value); }
            n++;
        }
        else if (elem == NORMAL && end) {
            if (n == 0) { mdl->normals[mdl->nnormals].x = value; }
            else if (n == 1) { mdl->normals[mdl->nnormals].y = value; }
            else {
                mdl->normals[mdl->nnormals].z = value;
                // .obj doesn't guarantee normalized
                vec3_unit_ip(mdl->normals + mdl->nnormals);
                mdl->nnormals++;
                if (mdl->nnormals >= mdl->cnormals) {
                    mdl->normals = SDL_realloc(
                        mdl->normals,
                        sizeof(vec3) * mdl->cnormals * ARR_FACTOR
                    );
                    if (mdl->normals == NULL) {
                        SDL_OutOfMemory();
                        return NULL;
                    }
                    mdl->cnormals *= ARR_FACTOR;
                }
            }
            n++;
        }
        else if (elem == UV) {
        }
        else if (elem == FACE) {
        }
    }
    // FREE UP DATA AFTER PARSING
    SDL_free(data);

    return mdl;
}

