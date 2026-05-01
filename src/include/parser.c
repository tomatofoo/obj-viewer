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
        SDL_free(data);
        SDL_OutOfMemory();
        return NULL;
    }
    // Using malloc instead of calloc because we have nvertices
    // malloc is adequate and faster
    mdl->nvertices = 0;
    mdl->vertices = SDL_malloc(sizeof(vec3) * ARR_SIZE);
    if (mdl->vertices == NULL) {
        SDL_free(data);
        SDL_free(mdl);
        SDL_OutOfMemory();
        return NULL;
    }
    mdl->cvertices = ARR_SIZE;
    mdl->nnormals = 0;
    mdl->normals = SDL_malloc(sizeof(vec3) * ARR_SIZE);
    if (mdl->normals == NULL) {
        SDL_free(data);
        SDL_free(mdl);
        SDL_free(mdl->vertices);
        SDL_OutOfMemory();
        return NULL;
    }
    mdl->cnormals = ARR_SIZE;
    mdl->nuvs = 0;
    mdl->uvs = SDL_malloc(sizeof(vec2) * ARR_SIZE);
    if (mdl->uvs == NULL) {
        SDL_free(data);
        SDL_free(mdl);
        SDL_free(mdl->vertices);
        SDL_free(mdl->normals);
        SDL_OutOfMemory();
        return NULL;
    }
    mdl->cuvs = ARR_SIZE;
    mdl->nfaces = 0;
    mdl->faces = SDL_malloc(sizeof(face) * ARR_SIZE);
    if (mdl->faces == NULL) {
        SDL_free(data);
        SDL_free(mdl);
        SDL_free(mdl->vertices);
        SDL_free(mdl->normals);
        SDL_free(mdl->faces);
        SDL_OutOfMemory();
        return NULL;
    }
    mdl->cfaces = ARR_SIZE;

    // PER ELEMENT
    etype elem = NONE;
    bool cont = false; // continue (e.g. comment, group, etc.)
    bool begin; // finished parsing element type; now will parse elem
    size_t n = 0; // index for arrays (resets at start)
    // PER ITEM AND MORE
    bool start = true; // start of new item (inside element)
    bool end; // if is last char of current item
    bool neg; // if number value (see below) is negative
    uint32_t whole; // whole part of value
    uint64_t decimal; // decmial part of value (not power)
    int dpower = -1; // power for decimal numbers
    bool eneg; // if epower is negative
    int epower = -1; // general power (for vertices with e in them)
    double value; // a number value (vertices, normals, etc.)
    size_t j; // index value
    size_t d; // an element inDex value (faces)
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
        if (data[i] == '#') { // comment
            cont = true;
            continue;
        }
        if (elem == NONE) {
            if (streq_space(data + i, "g")) { cont = true; }
            else if (streq_space(data + i, "o")) { cont = true; }
            else if (streq_space(data + i, "v")) { elem = VERTEX; }
            else if (streq_space(data + i, "vn")) { elem = NORMAL; }
            else if (streq_space(data + i, "vt")) { elem = UV; }
            else if (streq_space(data + i, "f")) { elem = FACE; }
            continue;
        }
        if (!begin) { continue; } // will start parsing after beginning
        end = isempty(data[i + 1]) || cont; // check if is end
        // Floating-point Number Parsing
        if (elem == VERTEX || elem == NORMAL || elem == UV) {
            if (start) {
                neg = false;
                whole = 0;
                decimal = 0;
                dpower = -1;
                epower = -1;
                value = 0;
                start = false;
                if (data[i] == '-') {
                    neg = true;
                    continue;
                }
            }
            if (data[i] == '.') {
                dpower = 0;
                continue;
            }
            if (data[i] == 'e') {
                eneg = false;
                epower = 0;
                continue;
            }
            if (epower > -1) {
                if (data[i] == '-') {
                    eneg = true;
                    continue;
                }
                epower = epower * 10 + (data[i] - '0');
            }
            else {
                if (dpower > -1) {
                    // This supports some pretty good precision
                    if (decimal < (SDL_MAX_UINT64 - 10) / 10) {
                        dpower++;
                        decimal = decimal * 10 + (data[i] - '0');
                    }
                }
                else { whole = whole * 10 + (data[i] - '0'); }
            }
            if (end) {
                value = whole + decimal / SDL_pow(10, dpower);
                if (epower > -1) { value *= SDL_pow(10, epower); }
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
            else if (n == 2) {
                mdl->normals[mdl->nnormals].z = value;
                // .obj doesn't guarantee normalized
                vec3_unit_ip(mdl->normals + mdl->nnormals);
                mdl->nnormals++;
                if (mdl->nnormals >= mdl->cnormals) {
                    mdl->normals = SDL_realloc(
                        mdl->normals, sizeof(vec3) * mdl->cnormals * ARR_FACTOR
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
        else if (elem == UV && end) {
            if (n == 0) {
                mdl->uvs[mdl->nuvs].x = value;
                mdl->uvs[mdl->nuvs].y = 0; // y is optional (default 0)
                mdl->nuvs++; // y is optional, so incrementing here
                if (mdl->nuvs >= mdl->cuvs) {
                    mdl->uvs = SDL_realloc(
                        mdl->uvs, sizeof(vec2) * mdl->cuvs * ARR_FACTOR
                    );
                    if (mdl->uvs == NULL) {
                        SDL_OutOfMemory();
                        return NULL;
                    }
                    mdl->cuvs *= ARR_FACTOR;
                }
            }
            // -1 because nuvs was incremented
            else if (n == 1) { mdl->uvs[mdl->nuvs - 1].y = value; }
            n++;
        }
        else if (elem == FACE) {
            if (start) {
                j = 0;
                d = 0;
                mdl->faces[mdl->nfaces].uvs[n] = -1;
                mdl->faces[mdl->nfaces].normals[n] = -1;
                if (n == 0) { mdl->faces[mdl->nfaces].texture = NULL; }
                start = false;
            }
            if (data[i] == '/') {
                if (j == 0) {
                    d = d >= 0 ? d - 1 : mdl->nvertices + d;
                    mdl->faces[mdl->nfaces].vertices[n] = d;
                }
                else if (j == 1) {
                    d = d >= 0 ? d - 1 : mdl->nuvs + d;
                    mdl->faces[mdl->nfaces].uvs[n] = d;
                }
                j++; // only need to increment in this if statement
                d = 0;
                continue;
            }
            d = d * 10 + (data[i] - '0');
            if (end) {
                // repeated; not sure if there is better way
                if (j == 0) {
                    d = d >= 0 ? d - 1 : mdl->nvertices + d;
                    mdl->faces[mdl->nfaces].vertices[n] = d;
                }
                else if (j == 1) {
                    d = d >= 0 ? d - 1 : mdl->nuvs + d;
                    mdl->faces[mdl->nfaces].uvs[n] = d;
                }
                else if (j == 2) {
                    d = d >= 0 ? d - 1 : mdl->nnormals + d;
                    mdl->faces[mdl->nfaces].normals[n] = d;
                }
                if (n == 2) {
                    mdl->faces[mdl->nfaces].centroid = vec3_add(vec3_add(
                        mdl->vertices[mdl->faces[mdl->nfaces].vertices[0]],
                        mdl->vertices[mdl->faces[mdl->nfaces].vertices[1]]),
                        mdl->vertices[mdl->faces[mdl->nfaces].vertices[2]]
                    );
                    vec3_div_ip(&mdl->faces[mdl->nfaces].centroid, 3);
                    // repurposing j
                    mdl->faces[mdl->nfaces].normal = (vec3) {0, 0, 0};
                    j = 0;
                    for (size_t k = 0; k < 3; k++) {
                        if (mdl->faces[mdl->nfaces].normals[k] == -1) {
                            continue;
                        }
                        vec3_add_ip(
                            &mdl->faces[mdl->nfaces].normal,
                            mdl->normals[mdl->faces[mdl->nfaces].normals[k]]
                        );
                        j++;
                    }
                    // ^ don't need to divide by j because normalizing anyway
                    if (j == 0) { // calculate normal vector using cross product
                        vec3 term1 = vec3_sub(
                            mdl->vertices[mdl->faces[mdl->nfaces].vertices[1]],
                            mdl->vertices[mdl->faces[mdl->nfaces].vertices[0]]
                        );
                        vec3 term2 = vec3_sub(
                            mdl->vertices[mdl->faces[mdl->nfaces].vertices[2]],
                            mdl->vertices[mdl->faces[mdl->nfaces].vertices[1]]
                        );
                        mdl->faces[mdl->nfaces].normal = vec3_cross(
                            term1, term2
                        );
                    }
                    vec3_unit_ip(&mdl->faces[mdl->nfaces].normal); // normalize
                    mdl->nfaces++;
                    if (mdl->nfaces >= mdl->cfaces) {
                        mdl->faces = SDL_realloc(
                            mdl->faces,
                            sizeof(face) * mdl->cfaces * ARR_FACTOR
                        );
                        if (mdl->faces == NULL) {
                            SDL_OutOfMemory();
                            return NULL;
                        }
                        mdl->cfaces *= ARR_FACTOR;
                    }
                }
                n++;
            }
        }
    }
    // FREE UP DATA AFTER PARSING
    SDL_free(data);

    return mdl;
}

