#include "SDL3/SDL.h"

#include "parser.h"
#include "renderer.h"
#include "utils.h"


#define ZBUF_RES 10000


typedef struct rface { // render face
    vec3 vertices[3];
    point points[3];
    vec2 uvs[3];
    vec3 normals[3];
} rface;


void destroy_model(model *mdl) {
    if (mdl == NULL) { return; }
    SDL_free(mdl->vertices);
    SDL_free(mdl->normals);
    SDL_free(mdl->uvs);
    SDL_free(mdl->faces);
    for (size_t i = 0; i < mdl->nmats; i++) {
        SDL_DestroySurface(mdl->mats[i].atexture);
        SDL_DestroySurface(mdl->mats[i].dtexture);
        SDL_DestroySurface(mdl->mats[i].stexture);
        SDL_DestroySurface(mdl->mats[i].gtexture);
    }
    SDL_free(mdl->mats);
    SDL_free(mdl);
}

bool normalize_model(model *mdl) {
    if (mdl == NULL) {
        SDL_SetError("Model cannot be NULL.");
        return false;
    }
    
    double largest = 0;
    for (size_t i = 0; i < mdl->nvertices; i++) {
        largest = SDL_max(vec3_mag(mdl->vertices[i].vec), largest);
    }
    if (largest == 0) { return true; }
    for (size_t i = 0; i < mdl->nvertices; i++) {
        vec3_div_ip(&mdl->vertices[i].vec, largest);
    }
    for (size_t i = 0; i < mdl->nfaces; i++) {
        vec3_div_ip(&mdl->faces[i].centroid, largest);
    }

    return true;
}

bool scale_model(model *mdl, double scale) {
    if (mdl == NULL) {
        SDL_SetError("Model cannot be NULL.");
        return false;
    }
    
    for (size_t i = 0; i < mdl->nvertices; i++) {
        vec3_mul_ip(&mdl->vertices[i].vec, scale);
    }
    for (size_t i = 0; i < mdl->nfaces; i++) {
        vec3_mul_ip(&mdl->faces[i].centroid, scale);
    }

    return true;
}


context *create_context(
    const char *path,
    SDL_Renderer *renderer,
    int w, int h
) {
    context *ctx = SDL_malloc(sizeof(context));
    if (ctx == NULL) {
        SDL_OutOfMemory();
        SDL_SetError(
            "Failed to allocate memory for context: %s", SDL_GetError()
        );
        return NULL;
    }
    // stacking error messages to help tracing
    ctx->mdl = parse_obj(path);
    if (ctx->mdl == NULL) {
        SDL_free(ctx);
        SDL_SetError(
            "Failed to parse OBJ file for context: %s", SDL_GetError()
        );
        return NULL;
    }
    ctx->proj = SDL_malloc(sizeof(point) * ctx->mdl->cvertices);
    if (ctx->proj == NULL) {
        destroy_model(ctx->mdl);
        SDL_free(ctx);
        SDL_SetError(
            "Failed to allocate memory for projected points: %s",
            SDL_GetError()
        );
        return NULL;
    }
    ctx->zbuf = SDL_malloc(sizeof(uint32_t) * w * h);
    if (ctx->zbuf == NULL) {
        destroy_model(ctx->mdl);
        SDL_free(ctx->proj);
        SDL_free(ctx);
        SDL_SetError(
            "Failed to allocate memory for z-buffer: %s", SDL_GetError()
        );
        return NULL;
    }
    ctx->pos = ZEROVEC3;
    ctx->rot = ZEROVEC3;
    ctx->flength = w / 2;
    ctx->near = 0.01;
    ctx->cull = true;
    ctx->blinn = true;
    ctx->quality = 3;
    ctx->mat = (material) {
        .ambient = (vec3) {0.1, 0.1, 0.1},
        .diffuse = (vec3) {0.5, 0.5, 0.5},
        .specular = (vec3) {1, 1, 1},
        .glossiness = 128,
        .transparency = 0,
        .atexture = NULL,
        .dtexture = NULL,
        .stexture = NULL,
        .gtexture = NULL,
    };
    ctx->brightness = -1;
    ctx->renderer = renderer;
    ctx->texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        w, h
    );
    if (ctx->texture == NULL) {
        destroy_model(ctx->mdl);
        SDL_free(ctx->proj);
        SDL_free(ctx->zbuf);
        SDL_free(ctx);
        SDL_SetError("Failed to create texture: %s", SDL_GetError());
        return NULL;
    }

    return ctx;
}

void destroy_context(context *ctx) {
    if (ctx == NULL) { return; }
    destroy_model(ctx->mdl);
    SDL_free(ctx->proj);
    SDL_free(ctx->zbuf);
    SDL_DestroySurface(ctx->mat.atexture);
    SDL_DestroySurface(ctx->mat.dtexture);
    SDL_DestroySurface(ctx->mat.stexture);
    SDL_DestroySurface(ctx->mat.gtexture);
    SDL_DestroyTexture(ctx->texture);
    SDL_free(ctx);
}

point project(context *ctx, vec3 rel) {
    double invz;
    invz = 1.0 / rel.z;
    return (point) {
        rel.x * invz * ctx->flength + ctx->texture->w / 2,
        -rel.y * invz * ctx->flength + ctx->texture->h / 2,
        rel,
        invz,
    };
}

void calc_mult(
    bool blinn,
    double brightness,
    double dot,
    vec3 *rel,
    vec3 *normal,
    vec3 *ambi_color,
    vec3 *diff_color,
    vec3 *spec_color,
    double glossiness,
    vec3 *ambi_ptr, // pointers to return values
    vec3 *diff_ptr,
    vec3 *spec_ptr
) {
    double invmag = 1.0 / vec3_mag(*rel);
    double specular_mult;
    vec3 diffuse = {0, 0, 0};
    vec3 specular = {0, 0, 0};
    if (!vec3_iszero(*diff_color)) {
        diffuse = vec3_mul(*diff_color, -dot * invmag);
    }
    if (!vec3_iszero(*spec_color)) {
        if (blinn) {
            // light source is camera
            // will have to change this when adding proper lighting
            specular_mult = SDL_pow(-dot * invmag, glossiness);
        }
        else {
            vec3 reflection = vec3_sub(*rel, vec3_mul(*normal, 2 * dot));
            specular_mult = SDL_pow(
                SDL_max(-vec3_dot(*rel, reflection) * invmag * invmag, 0),
                glossiness
            );
        }
        specular = vec3_mul(*spec_color, specular_mult);
    }
    vec3 ambient = *ambi_color;
    if (brightness != -1) {
        invmag *= brightness;
        vec3_mul_ip(&ambient, invmag);
        vec3_mul_ip(&diffuse, invmag);
        vec3_mul_ip(&specular, invmag);
    }
    *ambi_ptr = ambient;
    *diff_ptr = diffuse;
    *spec_ptr = specular;
}

vec3 read_pixel(
    model *mdl, size_t i, SDL_Surface *texture, double u, double v, double w
) {
    if (texture == NULL) { return (vec3) {1, 1, 1}; }
    int x = SDL_clamp((
        u * mdl->uvs[mdl->faces[i].uvs[0]].x
        + v * mdl->uvs[mdl->faces[i].uvs[1]].x
        + w * mdl->uvs[mdl->faces[i].uvs[2]].x
    ) * texture->w, 0, texture->w - 1);
    int y = SDL_clamp((
        u * mdl->uvs[mdl->faces[i].uvs[0]].y
        + v * mdl->uvs[mdl->faces[i].uvs[1]].y
        + w * mdl->uvs[mdl->faces[i].uvs[2]].y
    ) * texture->h, 0, texture->h - 1);
    uint8_t r;
    uint8_t g;
    uint8_t b;
    if (texture->pixels == NULL) {
        SDL_ReadSurfacePixel(texture, x, y, &r, &g, &b, NULL);
    }
    else { // expecting RGB24
        uint8_t *pixels = texture->pixels;
        size_t pixelsn = x * 3 + y * texture->pitch;
        r = pixels[pixelsn + 0];
        g = pixels[pixelsn + 1];
        b = pixels[pixelsn + 2];
    }
    return (vec3) {r / 255.0, g / 255.0, b / 255.0};
}

bool render(context *ctx, const SDL_FRect *srcrect, const SDL_FRect *dstrect) {
    uint8_t *pixels;
    int pitch;
    if (!SDL_LockTexture(ctx->texture, NULL, (void **) &pixels, &pitch)) {
        SDL_SetError("Failed to lock texture for writing: %s", SDL_GetError());
        return false;
    }

    // Clear Texture and z-buffer
    SDL_memset(pixels, 0, ctx->texture->h * pitch);
    SDL_memset(
        ctx->zbuf,
        SDL_MAX_UINT32,
        sizeof(uint32_t) * ctx->texture->w * ctx->texture->h
    );
    
    // Actual Rendering
    model *mdl = ctx->mdl;
    vec3 rel;
    for (size_t i = 0; i < mdl->nvertices; i++) {
        rel = vec3_sub(mdl->vertices[i].vec, ctx->pos);
        vec3_rot_y_ip(&rel, -ctx->rot.y);
        vec3_rot_x_ip(&rel, -ctx->rot.x);
        vec3_rot_z_ip(&rel, -ctx->rot.z);
        ctx->proj[i] = project(ctx, rel);
    }

    int xmin;
    int xmax;
    int ymin;
    int ymax;
    material *mat = &ctx->mat;
    double dot;
    double invmag;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 color; // buffer value
    vec3 mult = mat->ambient;
    double z; // reused in clipping
    point points[3]; // kinda shorthand thing
    vec2 diff10;
    vec2 diff20;
    double invdenom;
    // for clipping
    size_t ks[2]; // k-values used for clipping
    vec2 uvs[3];
    vec3 normals[3];
    rface faces[2]; // clipping faces
    double t; // lerp value for clipping
    size_t nfaces; // number of faces (1 or 2) to render
    size_t behind; // amount of vertices behind nearclip
    for (size_t i = 0; i < mdl->nfaces; i++) {
        // Clipping
        behind = 0;
        for (size_t j = 0; j < 3; j++) {
            z = ctx->proj[mdl->faces[i].vertices[j]].rel.z;
            if (z <= ctx->near) { behind++; }
        }
        if (behind == 3 || (behind != 0 && ctx->near == 0)) { continue; }
        for (size_t j = 0; j < 3; j++) {
            if (mdl->faces[i].normals[j] == -1) {
                normals[j] = mdl->vertices[mdl->faces[i].vertices[j]].normal;
            }
            else { normals[j] = mdl->normals[mdl->faces[i].normals[j]]; }
        }
        for (size_t j = 0; j < 3; j++) {
            if (mdl->faces[i].uvs[j] == -1) { uvs[j] = ZEROVEC2; }
            else { uvs[j] = mdl->uvs[mdl->faces[i].uvs[j]]; }
        }
        if (behind == 2) { // TODO: FIX UV LERPING
            nfaces = 1;
            for (size_t j = 0; j < 3; j++) {
                z = ctx->proj[mdl->faces[i].vertices[j]].rel.z;
                if (z <= ctx->near) { continue; }
                if (j == 0) {
                    ks[0] = 1;
                    ks[1] = 2;
                }
                else if (j == 1) {
                    ks[0] = 2;
                    ks[1] = 0;
                }
                else {
                    ks[0] = 0;
                    ks[1] = 1;
                }
                // original
                faces[0].vertices[j] = (
                    mdl->vertices[mdl->faces[i].vertices[j]].vec
                );
                faces[0].points[j] = ctx->proj[mdl->faces[i].vertices[j]];
                faces[0].uvs[j] = mdl->uvs[mdl->faces[i].uvs[j]];
                faces[0].normals[j] = normals[j];
                // lerp
                for (size_t k = 0; k < 2; k++) {
                    rel = ctx->proj[mdl->faces[i].vertices[ks[k]]].rel;
                    t = (ctx->near - rel.z) / (z - rel.z);
                    faces[0].vertices[ks[k]] = vec3_lerp(
                        mdl->vertices[mdl->faces[i].vertices[ks[k]]].vec,
                        mdl->vertices[mdl->faces[i].vertices[j]].vec,
                        t
                    );
                    vec3_lerp_ip(
                        &rel, ctx->proj[mdl->faces[i].vertices[j]].rel, t
                    );
                    faces[0].points[ks[k]] = project(ctx, rel);
                    faces[0].uvs[ks[k]] = vec2_lerp(uvs[ks[k]], uvs[j], t);
                    // not normalized until the end
                    faces[0].normals[ks[k]] = vec3_lerp(
                        normals[ks[k]], normals[j], t
                    );
                }
            }
        }
        else if (behind == 1) {
            nfaces = 2;
            for (size_t j = 0; j < 3; j++) {
                z = ctx->proj[mdl->faces[i].vertices[j]].rel.z;
                if (z > ctx->near) { continue; }
                if (j == 0) {
                    ks[0] = 1;
                    ks[1] = 2;
                }
                else if (j == 1) {
                    ks[0] = 2;
                    ks[1] = 0;
                }
                else {
                    ks[0] = 0;
                    ks[1] = 1;
                }
                // face[0]
                for (size_t k = 0; k < 2; k++) {
                    faces[0].vertices[ks[k]] = (
                        mdl->vertices[mdl->faces[i].vertices[ks[k]]].vec
                    );
                    faces[0].points[ks[k]] = (
                        ctx->proj[mdl->faces[i].vertices[ks[k]]]
                    );
                    faces[0].uvs[ks[k]] = mdl->uvs[mdl->faces[i].uvs[ks[k]]];
                    faces[0].normals[ks[k]] = normals[ks[k]];
                }
                rel = ctx->proj[mdl->faces[i].vertices[ks[1]]].rel;
                t = (rel.z - ctx->near) / (rel.z - z);
                faces[0].vertices[j] = vec3_lerp(
                    mdl->vertices[mdl->faces[i].vertices[ks[1]]].vec,
                    mdl->vertices[mdl->faces[i].vertices[j]].vec,
                    t
                );
                vec3_lerp_ip(
                    &rel, ctx->proj[mdl->faces[i].vertices[j]].rel, t
                );
                faces[0].points[j] = project(ctx, rel);
                faces[0].uvs[j] = vec2_lerp(uvs[ks[1]], uvs[j], t);
                faces[0].normals[j] = vec3_lerp(normals[ks[1]], normals[j], t);
                // face[1]
                faces[1].vertices[ks[0]] = faces[0].vertices[j];
                faces[1].points[ks[0]] = faces[0].points[j];
                faces[1].uvs[ks[0]] = faces[0].uvs[j];
                faces[1].normals[ks[0]] = faces[0].normals[j];
                rel = ctx->proj[mdl->faces[i].vertices[ks[0]]].rel;
                t = (rel.z - ctx->near) / (rel.z - z);
                faces[1].vertices[ks[1]] = vec3_lerp(
                    mdl->vertices[mdl->faces[i].vertices[ks[0]]].vec,
                    mdl->vertices[mdl->faces[i].vertices[j]].vec,
                    t
                );
                vec3_lerp_ip(
                    &rel, ctx->proj[mdl->faces[i].vertices[j]].rel, t
                );
                faces[1].points[ks[1]] = project(ctx, rel);
                faces[1].uvs[ks[1]] = vec2_lerp(uvs[ks[0]], uvs[j], t);
                faces[1].normals[ks[1]] = vec3_lerp(normals[ks[0]], normals[j], t);
                faces[1].vertices[j] = (
                    mdl->vertices[mdl->faces[i].vertices[ks[0]]].vec
                );
                faces[1].points[j] = ctx->proj[mdl->faces[i].vertices[ks[0]]];
                faces[1].uvs[j] = mdl->uvs[mdl->faces[i].uvs[ks[0]]];
                faces[1].normals[j] = normals[ks[0]];
            }
        }
        else {
            nfaces = 1;
            faces[0] = (rface) {
                .vertices = {
                    mdl->vertices[mdl->faces[i].vertices[0]].vec,
                    mdl->vertices[mdl->faces[i].vertices[1]].vec,
                    mdl->vertices[mdl->faces[i].vertices[2]].vec
                },
                .points = {
                    ctx->proj[mdl->faces[i].vertices[0]],
                    ctx->proj[mdl->faces[i].vertices[1]],
                    ctx->proj[mdl->faces[i].vertices[2]]
                },
                .uvs = {uvs[0], uvs[1], uvs[2]},
                .normals = {normals[0], normals[1], normals[2]},
            };
        }
        // Actual rendering
        for (size_t j = 0; j < nfaces; j++) {
            rel = vec3_sub(mdl->faces[i].centroid, ctx->pos);
            dot = vec3_dot(rel, mdl->faces[i].normal);
            if (ctx->cull && dot > 0) { continue; } // Backface culling
           
            // Load material
            if (mdl->faces[i].mat == -1) { mat = &ctx->mat; }
            else { mat = mdl->mats + mdl->faces[i].mat; }

            if (ctx->quality == 1) {
                // Lighting (Phong lighting)
                calc_mult(
                    ctx->blinn,
                    ctx->brightness,
                    dot,
                    &rel,
                    &mdl->faces[i].normal,
                    &mat->ambient,
                    &mat->diffuse,
                    &mat->specular,
                    mat->glossiness,
                    &ambient,
                    &diffuse,
                    &specular
                );
            }

            // Get triangle bounds
            xmin = ctx->texture->w;
            xmax = 0;
            ymin = ctx->texture->h;
            ymax = 0;
            for (size_t k = 0; k < 3; k++) {
                points[k] = faces[j].points[k];
                xmin = SDL_min(SDL_max(points[k].x, 0), xmin);
                xmax = SDL_max(SDL_min(points[k].x, ctx->texture->w), xmax);
                ymin = SDL_min(SDL_max(points[k].y, 0), ymin);
                ymax = SDL_max(SDL_min(points[k].y, ctx->texture->h), ymax);
            }

            // Caching some stuff for barycentric calculations
            diff10.x = points[1].x - points[0].x;
            diff10.y = points[1].y - points[0].y;
            diff20.x = points[2].x - points[0].x;
            diff20.y = points[2].y - points[0].y;
            invdenom = 1.0 / (diff10.x * diff20.y - diff20.x * diff10.y);

            // Half-space triangle checking
            // https://sw-shader.sourceforge.net/rasterizer.html
            // ^ use Wayback Machine
            // assumes counter-clockwise vertex order
            int xdiff[] = {
                points[1].x - points[0].x,
                points[2].x - points[1].x,
                points[0].x - points[2].x
            };
            int ydiff[] = {
                points[1].y - points[0].y,
                points[2].y - points[1].y,
                points[0].y - points[2].y
            };
            // Expressions that get added to/subtracted from
            int xexp[3];
            int yexp[3];
            for (size_t k = 0; k < 3; k++) {
                yexp[k] = (
                    xdiff[k] * (ymin - points[k].y)
                    - ydiff[k] * (xmin - points[k].x)
                    + (ydiff[k] < 0 || (ydiff[k] == 0 && xdiff[k] > 0))
                );
            }

            size_t zbufy = ymin * ctx->texture->w + xmin;
            size_t pixelsy = ymin * pitch + xmin * 3;
            size_t zbufn;
            size_t pixelsn;
            vec2 diffx0;
            double u;
            double v;
            double w;
            vec3 normal = mdl->faces[i].normal;
            for (int y = ymin; y < ymax; y++) {
                zbufn = zbufy;
                pixelsn = pixelsy;
                xexp[0] = yexp[0];
                xexp[1] = yexp[1];
                xexp[2] = yexp[2];
                for (int x = xmin; x < xmax; x++) {
                    // half-space check
                    if (xexp[0] > 0 && xexp[1] > 0 && xexp[2] > 0) {
                        // Compute barycentric coordinates in screen space
                        diffx0 = (vec2) {x - points[0].x, y - points[0].y};
                        v = (
                            diffx0.x * diff20.y - diff20.x * diffx0.y
                        ) * invdenom;
                        w = (
                            diff10.x * diffx0.y - diffx0.x * diff10.y
                        ) * invdenom;
                        u = 1.0 - v - w;
                        // Correct perspective
                        u *= points[0].invz;
                        v *= points[1].invz;
                        w *= points[2].invz;
                        invmag = 1.0 / (u + v + w);
                        u *= invmag;
                        v *= invmag;
                        w *= invmag;
                        // not using continue because it will not do subtract
                        z = (
                            u * points[0].rel.z
                            + v * points[1].rel.z
                            + w * points[2].rel.z
                        );
                        if (z * ZBUF_RES < ctx->zbuf[zbufn]) {
                            ctx->zbuf[zbufn] = (uint32_t) (z * ZBUF_RES);
                            // per-pixel lighting
                            if (ctx->quality > 1) {
                                rel = vec3_mul(faces[j].vertices[0], u);
                                vec3_add_ip(&rel, vec3_mul(
                                    faces[j].vertices[1], v
                                ));
                                vec3_add_ip(&rel, vec3_mul(
                                    faces[j].vertices[2], w 
                                ));
                                vec3_sub_ip(&rel, ctx->pos);
                                if (ctx->quality > 2) {
                                    normal = vec3_unit(vec3_add(vec3_add(
                                        vec3_mul(faces[j].normals[0], u),
                                        vec3_mul(faces[j].normals[1], v)),
                                        vec3_mul(faces[j].normals[2], w))
                                    );
                                }
                                dot = vec3_dot(rel, normal);
                                color = read_pixel(
                                    mdl, i, mat->gtexture, u, v, w
                                );
                                calc_mult(
                                    ctx->blinn,
                                    ctx->brightness,
                                    dot,
                                    &rel,
                                    &normal,
                                    &mat->ambient, 
                                    &mat->diffuse,
                                    &mat->specular,
                                    mat->glossiness
                                    * (color.x + color.y + color.z) / 3,
                                    &ambient,
                                    &diffuse,
                                    &specular
                                );
                            }
                            if (ctx->quality > 0) {
                                color = read_pixel(
                                    mdl, i, mat->atexture, u, v, w
                                );
                                mult.x = color.x * ambient.x;
                                mult.y = color.y * ambient.y;
                                mult.z = color.z * ambient.z;
                                color = read_pixel(
                                    mdl, i, mat->dtexture, u, v, w
                                );
                                mult.x += color.x * diffuse.x;
                                mult.y += color.y * diffuse.y;
                                mult.z += color.z * diffuse.z;
                                color = read_pixel(
                                    mdl, i, mat->stexture, u, v, w
                                );
                                mult.x += color.x * specular.x;
                                mult.y += color.y * specular.y;
                                mult.z += color.z * specular.z;
                                mult.x = SDL_max(mult.x, 0);
                                mult.y = SDL_max(mult.y, 0);
                                mult.z = SDL_max(mult.z, 0);
                            }
                            pixels[pixelsn + 0] = SDL_min(mult.x * 255, 255);
                            pixels[pixelsn + 1] = SDL_min(mult.y * 255, 255);
                            pixels[pixelsn + 2] = SDL_min(mult.z * 255, 255);
                        }
                    }
                    zbufn++;
                    pixelsn += 3;
                    for (size_t k = 0; k < 3; k++) { xexp[k] -= ydiff[k]; }
                }
                zbufy += ctx->texture->w;
                pixelsy += pitch;
                for (size_t k = 0; k < 3; k++) { yexp[k] += xdiff[k]; }
            }
        }
    }

    SDL_UnlockTexture(ctx->texture);
    SDL_RenderTexture(ctx->renderer, ctx->texture, srcrect, dstrect);

    return true;
}

