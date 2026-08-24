#include "SDL3/SDL.h"

#include "parser.h"
#include "renderer.h"
#include "utils.h"


#define ARR_SIZE 8 // begninning array size for model and mats
#define ARR_FACTOR 2 // factor when resizing


typedef enum etype { // Element Types for both OBj and MTL
    NONE,
    // OBJ
    MATLIB,
    MAT,
    VERTEX,
    NORMAL,
    UV,
    FACE,
    // MTL
    NEWMAT,
    AMB,
    DIFF,
    SPEC,
    GLOSS,
    ATEX,
    DTEX,
    STEX,
    GTEX,
} etype;

typedef struct uface { // unconverted face (could be quad)
    // all are indices
    size_t vertices[4];
    vec3 centroid;
    int32_t uvs[4]; // int because need -1
    int32_t normals[4];
    vec3 normal; // average of all normals
} uface;

// based on https://benhoyt.com/writings/hash-table-in-c/
typedef struct mentry {
    char *key;
    material mat;
} mentry;

typedef struct mtable {
    mentry *entries;
    size_t nentries;
    size_t centries;
} mtable;


mtable *create_mtable() {
    mtable *mt = SDL_malloc(sizeof(mtable));
    if (mt == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }
    mt->entries = SDL_calloc(ARR_SIZE, sizeof(mentry));
    if (mt->entries == NULL) {
        SDL_free(mt);
        SDL_OutOfMemory();
        return NULL;
    }
    mt->nentries = 0;
    mt->centries = ARR_SIZE;

    return mt;
}

void destroy_mtable(mtable *mt) {
    for (size_t i = 0; i < mt->centries; i++) {
        SDL_free(mt->entries[i].key); // can free NULL
    }
    SDL_free(mt->entries);
    SDL_free(mt);
}

bool mtable_set(mtable *mt, char *key, material mat) {
    size_t i = fnv1a32(key) % mt->centries;
    while (mt->entries[i].key != NULL) { i = (i + 1) % mt->centries; }
    mt->entries[i].key = key; // expecting to be provided malloced
    mt->entries[i].mat = mat;
    mt->nentries++;
    if (mt->nentries >= mt->centries) {
        mt->centries *= ARR_FACTOR;
        mentry *entries = SDL_calloc(mt->centries, sizeof(mentry));
        if (entries == NULL) {
            SDL_OutOfMemory();
            return false;
        }
        size_t j;
        for (size_t i = 0; i < mt->centries; i++) {
            if (mt->entries[i].key == NULL) { continue; }
            j = fnv1a32(mt->entries[i].key) % mt->centries;
            while (entries[j].key != NULL) { j = (j + 1) % mt->centries; }
            entries[j].key = mt->entries[i].key;
            entries[j].mat = mt->entries[i].mat;
        }
        SDL_free(mt->entries);
        mt->entries = entries;
    }
    return true;
}

material *mtable_get(mtable *mt, char *key) {
    if (key == NULL) { return NULL; }
    size_t i = fnv1a32(key) % mt->centries;
    for (size_t j = 0; mt->entries[i].key != NULL && j < mt->centries; j++) {
        if (SDL_strcmp(mt->entries[i].key, key)) {
            i = (i + 1) % mt->centries;
        }
        else { return &mt->entries[i].mat; }
    }
    return NULL;
}


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


bool parse_mtl(const char *path, mtable *mt) {
    const char *ext = filename_lext(path);
    if (!(SDL_strcmp(ext, "mtl") == 0 || SDL_strcmp(ext, "MTL") == 0)) {
        SDL_SetError("Filename extension is not mtl or MTL");
        return false;
    }
    size_t datasize;
    char *data = SDL_LoadFile(path, &datasize);
    if (data == NULL) {
        SDL_SetError("Failed to load MTL file: %s", SDL_GetError());
        return false;
    }

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
    int32_t dpower = -1; // power for decimal numbers
    bool eneg; // if epower is negative
    int32_t epower = -1; // general power (for values with e in them)
    double value; // a number value (factor, exponent, etc.)
    size_t j; // index value

    // FREE UP DATA AFTER PARSING
    SDL_free(data);

    return true;
}

model *parse_obj(const char *path) {
    const char *ext = filename_lext(path);
    if (!(SDL_strcmp(ext, "obj") == 0 || SDL_strcmp(ext, "OBJ") == 0)) {
        SDL_SetError("Filename extension is not obj or OBJ");
        return NULL;
    }
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
    mdl->vertices = SDL_malloc(sizeof(vertex) * ARR_SIZE);
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
        SDL_free(mdl->uvs);
        SDL_OutOfMemory();
        return NULL;
    }
    mdl->cfaces = ARR_SIZE;
    mdl->nmats = 0;
    mdl->mats = SDL_malloc(sizeof(material) * ARR_SIZE);
    if (mdl->mats == NULL) {
        SDL_free(data);
        SDL_free(mdl);
        SDL_free(mdl->vertices);
        SDL_free(mdl->normals);
        SDL_free(mdl->uvs);
        SDL_free(mdl->faces);
        SDL_OutOfMemory();
        return NULL;
    }
    mdl->cmats = ARR_SIZE;

    // PER ELEMENT
    etype elem = NONE;
    uface rface; // raw face; used when parsing faces
    bool cont = false; // continue (e.g. comment, group, etc.)
    bool begin; // finished parsing element type; now will parse elem
    size_t n = 0; // index for arrays (resets at start)
    int32_t mat = -1; // material index for faces
    // PER ITEM AND MORE
    bool start = true; // start of new item (inside element)
    bool end; // if is last char of current item
    bool neg; // if number value (see below) is negative
    uint32_t whole; // whole part of value
    uint64_t decimal; // decmial part of value (not power)
    int32_t dpower = -1; // power for decimal numbers
    bool eneg; // if epower is negative
    int32_t epower = -1; // general power (for vertices with e in them)
    double value; // a number value (vertices, normals, etc.)
    size_t j; // index value
    int32_t d; // an element inDex value (faces) (int because need -1)
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
            else if (streq_space(data + i, "mtllib")) { elem = MATLIB; cont = true; } // TEMP
            else if (streq_space(data + i, "usemtl")) { elem = MAT; cont = true; } // TEMP
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
                value = 0.0;
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
            if (data[i] == 'e' || data[i] == 'E') {
                eneg = false;
                epower = 0;
                continue;
            }
            if (!(SDL_isdigit(data[i]) || data[i] == '-')) {
                SDL_SetError("Invalid digits received");
                goto invalid;
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
                    if (decimal < (SDL_MAX_UINT64 - 10) * 0.1) {
                        dpower++;
                        decimal = decimal * 10 + (data[i] - '0');
                    }
                }
                else { whole = whole * 10 + (data[i] - '0'); }
            }
            if (end) {
                value = whole + decimal / SDL_pow(10, dpower);
                if (epower > -1) {
                    if (eneg) { epower = -epower; }
                    value *= SDL_pow(10, epower);
                }
                if (neg) { value = -value; }
            }
        }
        if (elem == VERTEX && end) { // don't need to initialize item in array
            if (n == 0) { mdl->vertices[mdl->nvertices].vec.x = value; }
            else if (n == 1) { mdl->vertices[mdl->nvertices].vec.y = value; }
            else if (n == 2) {
                mdl->vertices[mdl->nvertices].normal = (vec3) {0, 0, 0};
                mdl->vertices[mdl->nvertices].vec.z = value;
                mdl->nvertices++;
                if (mdl->nvertices >= mdl->cvertices) {
                    mdl->vertices = SDL_realloc(
                        mdl->vertices,
                        sizeof(vertex) * mdl->cvertices * ARR_FACTOR
                    );
                    if (mdl->vertices == NULL) {
                        SDL_OutOfMemory();
                        return NULL;
                    }
                    mdl->cvertices *= ARR_FACTOR;
                }
            }
            // w is scaling divisor
            else {
                vec3_div_ip(&mdl->vertices[mdl->nvertices - 1].vec, value);
            }
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
                rface.uvs[n] = -1;
                rface.normals[n] = -1;
                start = false;
            }
            if (data[i] == '/') {
                if (j == 0) {
                    d = d >= 0 ? d - 1 : mdl->nvertices + d;
                    if (d < 0 || d >= mdl->nvertices) {
                        SDL_SetError("Invalid vertex index received");
                        goto invalid;
                    }
                    rface.vertices[n] = d;
                }
                else if (j == 1) {
                    d = d >= 0 ? d - 1 : mdl->nuvs + d;
                    // avoid negative and postive size_t comparison stuff
                    if (d < -1 || d >= (int32_t) mdl->nuvs) {
                        SDL_SetError("Invalid UV index received");
                        goto invalid;
                    }
                    rface.uvs[n] = d;
                }
                j++; // only need to increment in this if statement
                d = 0;
                continue;
            }
            if (!SDL_isdigit(data[i])) {
                SDL_SetError("Invalid digits received");
                goto invalid;
            }
            d = d * 10 + (data[i] - '0');
            if (end) {
                // repeated; not sure if there is better way
                if (j == 0) {
                    d = d >= 0 ? d - 1 : mdl->nvertices + d;
                    if (d < 0 || d >= mdl->nvertices) {
                        SDL_SetError("Invalid vertex index received");
                        goto invalid;
                    }
                    rface.vertices[n] = d;
                }
                else if (j == 1) {
                    d = d >= 0 ? d - 1 : mdl->nuvs + d;
                    if (d < -1 || d >= (int32_t) mdl->nuvs) {
                        SDL_SetError("Invalid UV index received");
                        goto invalid;
                    }
                    rface.uvs[n] = d;
                }
                else if (j == 2) {
                    d = d >= 0 ? d - 1 : mdl->nnormals + d;
                    if (d < -1 || d >= (int32_t) mdl->nnormals) {
                        SDL_SetError("Invalid normal index received");
                        goto invalid;
                    }
                    rface.normals[n] = d;
                }
                if (n == 2) {
                    mdl->faces[mdl->nfaces].centroid = vec3_add(vec3_add(
                        mdl->vertices[rface.vertices[0]].vec,
                        mdl->vertices[rface.vertices[1]].vec),
                        mdl->vertices[rface.vertices[2]].vec
                    );
                    vec3_div_ip(&mdl->faces[mdl->nfaces].centroid, 3);
                    // repurposing j
                    mdl->faces[mdl->nfaces].normal = (vec3) {0, 0, 0};
                    j = 0;
                    for (size_t k = 0; k < 3; k++) {
                        if (rface.normals[k] == -1) { continue; }
                        vec3_add_ip(
                            &mdl->faces[mdl->nfaces].normal,
                            mdl->normals[rface.normals[k]]
                        );
                        j++;
                    }
                    // ^ don't need to divide by j because normalizing anyway
                    if (j < 3) { // calculate normal vector using cross product
                        vec3 term1 = vec3_sub(
                            mdl->vertices[rface.vertices[1]].vec,
                            mdl->vertices[rface.vertices[0]].vec
                        );
                        vec3 term2 = vec3_sub(
                            mdl->vertices[rface.vertices[2]].vec,
                            mdl->vertices[rface.vertices[1]].vec
                        );
                        rface.normal = vec3_unit(vec3_cross(term1, term2));
                        mdl->faces[mdl->nfaces].normal = rface.normal;
                        for (size_t k = 0; k < 3; k++) {
                            if (SDL_isnan(rface.normal.x)
                                || SDL_isnan(rface.normal.y)
                                || SDL_isnan(rface.normal.z)) {
                                continue;
                            } // i spent weeks debugging this
                            vec3_add_ip(
                                &mdl->vertices[rface.vertices[k]].normal,
                                rface.normal
                            );
                        }
                    }
                    else { vec3_unit_ip(&mdl->faces[mdl->nfaces].normal); }
                    mdl->faces[mdl->nfaces].vertices[0] = rface.vertices[0];
                    mdl->faces[mdl->nfaces].vertices[1] = rface.vertices[1];
                    mdl->faces[mdl->nfaces].vertices[2] = rface.vertices[2];
                    mdl->faces[mdl->nfaces].uvs[0] = rface.uvs[0];
                    mdl->faces[mdl->nfaces].uvs[1] = rface.uvs[1];
                    mdl->faces[mdl->nfaces].uvs[2] = rface.uvs[2];
                    mdl->faces[mdl->nfaces].normals[0] = rface.normals[0];
                    mdl->faces[mdl->nfaces].normals[1] = rface.normals[1];
                    mdl->faces[mdl->nfaces].normals[2] = rface.normals[2];
                    mdl->faces[mdl->nfaces].mat = mat;
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
                // quadrilateral splitting (only works with convex quads)
                // split into 012 and 230
                else if (n == 3) {
                    rface.centroid = vec3_add(vec3_add(vec3_add(
                        mdl->vertices[rface.vertices[0]].vec,
                        mdl->vertices[rface.vertices[1]].vec),
                        mdl->vertices[rface.vertices[2]].vec),
                        mdl->vertices[rface.vertices[3]].vec
                    );
                    vec3_div_ip(&rface.centroid, 4);
                    // copy the normal in case j is 0
                    vec3 normal = rface.normal;
                    rface.normal = (vec3) {0, 0, 0};
                    j = 0;
                    for (size_t k = 0; k < 4; k++) {
                        if (rface.normals[k] == -1) { continue; }
                        vec3_add_ip(
                            &rface.normal, mdl->normals[rface.normals[k]]
                        );
                        j++;
                    }
                    if (j < 3) {
                        vec3 term1 = vec3_sub(
                            mdl->vertices[rface.vertices[3]].vec,
                            mdl->vertices[rface.vertices[2]].vec
                        );
                        vec3 term2 = vec3_sub(
                            mdl->vertices[rface.vertices[0]].vec,
                            mdl->vertices[rface.vertices[3]].vec
                        );
                        // average both cross product normals
                        rface.normal = vec3_unit(vec3_add(
                            normal, vec3_unit(vec3_cross(term1, term2))
                        ));
                        for (size_t k = 0; k < 3; k++) {
                            vec3_sub_ip( // remove old normal from overall
                                &mdl->vertices[rface.vertices[k]].normal,
                                normal
                            );
                        }
                        for (size_t k = 0; k < 4; k++) {
                            if (SDL_isnan(rface.normal.x)
                                || SDL_isnan(rface.normal.y)
                                || SDL_isnan(rface.normal.z)) {
                                continue;
                            }
                            vec3_add_ip(
                                &mdl->vertices[rface.vertices[k]].normal,
                                rface.normal
                            );
                        }
                    }
                    else { vec3_unit_ip(&rface.normal); }
                    mdl->faces[mdl->nfaces - 1].centroid = rface.centroid;
                    mdl->faces[mdl->nfaces - 1].normal = rface.normal;
                    mdl->faces[mdl->nfaces].centroid = rface.centroid;
                    mdl->faces[mdl->nfaces].normal = rface.normal;
                    mdl->faces[mdl->nfaces].vertices[0] = rface.vertices[2];
                    mdl->faces[mdl->nfaces].vertices[1] = rface.vertices[3];
                    mdl->faces[mdl->nfaces].vertices[2] = rface.vertices[0];
                    mdl->faces[mdl->nfaces].uvs[0] = rface.uvs[2];
                    mdl->faces[mdl->nfaces].uvs[1] = rface.uvs[3];
                    mdl->faces[mdl->nfaces].uvs[2] = rface.uvs[0];
                    mdl->faces[mdl->nfaces].normals[0] = rface.normals[2];
                    mdl->faces[mdl->nfaces].normals[1] = rface.normals[3];
                    mdl->faces[mdl->nfaces].normals[2] = rface.normals[0];
                    mdl->faces[mdl->nfaces].mat = mat;
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
    // normalize all vertex normals
    for (size_t i = 0; i < mdl->nvertices; i++) {
        if (!vec3_iszero(mdl->vertices[i].normal)) {
            vec3_unit_ip(&mdl->vertices[i].normal);
        }
    }
    // FREE UP DATA AFTER PARSING
    SDL_free(data);

    return mdl;

invalid: // invalid data
    SDL_free(data);
    SDL_free(mdl);
    SDL_free(mdl->vertices);
    SDL_free(mdl->normals);
    SDL_free(mdl->uvs);
    SDL_free(mdl->faces);
    SDL_free(mdl->mats);
    SDL_SetError("Invalid OBJ data: %s", SDL_GetError());
    return NULL;
}

