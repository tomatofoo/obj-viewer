#include "SDL3/SDL.h"

#include "parser.h"
#include "renderer.h"
#include "utils.h"


#define ZBUF_RES 10000


void destroy_model(model *mdl) {
    if (mdl == NULL) { return; }
    SDL_free(mdl->vertices);
    SDL_free(mdl->normals);
    SDL_free(mdl->uvs);
    SDL_free(mdl->faces);
    for (size_t i = 0; i < mdl->nmats; i++) {
        SDL_DestroySurface(mdl->mats[i].texture);
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
    
    for (size_t i; i < mdl->nvertices; i++) {
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
            "Failed allocate memory for projected points: %s", SDL_GetError()
        );
        return NULL;
    }
    ctx->zbuf = SDL_malloc(sizeof(uint32_t) * w * h);
    if (ctx->zbuf == NULL) {
        destroy_model(ctx->mdl);
        SDL_free(ctx->proj);
        SDL_free(ctx);
        SDL_SetError(
            "Failed allocate memory for z-buffer: %s", SDL_GetError()
        );
        return NULL;
    }
    ctx->pos = (vec3) {0, 0, 0};
    ctx->rot = (vec3) {0, 0, 0};
    ctx->flength = w / 2;
    ctx->blinn = true;
    ctx->quality = 3;
    ctx->ambient = (vec3) {0, 0, 0};
    ctx->diffuse = (vec3) {1, 1, 1};
    ctx->specular = (vec3) {0, 0, 0};
    ctx->glossiness = 16;
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
    SDL_DestroyTexture(ctx->texture);
    SDL_free(ctx);
}

vec3 calc_mult(
    bool blinn,
    double brightness,
    double dot,
    vec3 *rel,
    vec3 *normal,
    vec3 *ambi_color,
    vec3 *diff_color,
    vec3 *spec_color,
    double glossiness
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
    vec3 mult = vec3_add(vec3_add(*ambi_color, diffuse), specular);
    if (brightness != -1) { vec3_mul_ip(&mult, invmag * brightness); }
    mult.x = SDL_max(mult.x, 0);
    mult.y = SDL_max(mult.y, 0);
    mult.z = SDL_max(mult.z, 0);

    return mult;
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
        if (rel.z <= 0) {
            ctx->proj[i] = (point) {0, 0, -1, -1};
            continue;
        }
        ctx->proj[i] = (point) {
            rel.x / rel.z * ctx->flength + ctx->texture->w / 2,
            -rel.y / rel.z * ctx->flength + ctx->texture->h / 2,
            rel.z,
            1.0 / rel.z,
        };
    }

    int xmin;
    int xmax;
    int ymin;
    int ymax;
    double dot;
    double invmag;
    vec3 mult = {1, 1, 1};
    double z;
    point points[3];
    vec2 diff10;
    vec2 diff20;
    double invdenom;
    for (size_t i = 0; i < mdl->nfaces; i++) {
        // Culling
        if (ctx->proj[mdl->faces[i].vertices[0]].z < 0) { continue; }
        if (ctx->proj[mdl->faces[i].vertices[1]].z < 0) { continue; }
        if (ctx->proj[mdl->faces[i].vertices[2]].z < 0) { continue; }
        rel = vec3_sub(mdl->faces[i].centroid, ctx->pos);
        dot = vec3_dot(rel, mdl->faces[i].normal);
        if (dot > 0) { continue; } // Backface culling
        
        if (ctx->quality == 1) {
            // Lighting (Phong lighting)
            mult = calc_mult(
                ctx->blinn,
                ctx->brightness,
                dot,
                &rel,
                &mdl->faces[i].normal,
                &ctx->ambient,
                &ctx->diffuse,
                &ctx->specular,
                ctx->glossiness
            );
        }

        // Get triangle bounds
        xmin = ctx->texture->w;
        xmax = 0;
        ymin = ctx->texture->h;
        ymax = 0;
        for (size_t j = 0; j < 3; j++) {
            points[j] = ctx->proj[mdl->faces[i].vertices[j]];
            xmin = SDL_min(SDL_max(points[j].x, 0), xmin);
            xmax = SDL_max(SDL_min(points[j].x, ctx->texture->w), xmax);
            ymin = SDL_min(SDL_max(points[j].y, 0), ymin);
            ymax = SDL_max(SDL_min(points[j].y, ctx->texture->h), ymax);
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
        for (size_t j = 0; j < 3; j++) {
            yexp[j] = (
                xdiff[j] * (ymin - points[j].y)
                - ydiff[j] * (xmin - points[j].x)
                + (ydiff[j] < 0 || (ydiff[j] == 0 && xdiff[j] > 0))
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
        vec3 normals[3];
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
                    v = (diffx0.x * diff20.y - diff20.x * diffx0.y) * invdenom;
                    w = (diff10.x * diffx0.y - diffx0.x * diff10.y) * invdenom;
                    u = 1.0 - v - w;
                    // Correct perspective
                    u *= points[0].invz;
                    v *= points[1].invz;
                    w *= points[2].invz;
                    invmag = 1.0 / (u + v + w);
                    u *= invmag;
                    v *= invmag;
                    w *= invmag;
                    // not using continue because it will not do subtraction
                    z = u * points[0].z + v * points[1].z + w * points[2].z;
                    // per-pixel lighting
                    if (ctx->quality > 1) {
                        rel = vec3_sub(vec3_add(vec3_add(
                            vec3_mul(
                                mdl->vertices[mdl->faces[i].vertices[0]].vec, u
                            ),
                            vec3_mul(
                                mdl->vertices[mdl->faces[i].vertices[1]].vec, v
                            )),
                            vec3_mul(
                                mdl->vertices[mdl->faces[i].vertices[2]].vec, w
                            )),
                            ctx->pos
                        );
                        if (ctx->quality > 2) {
                            for (size_t j = 0; j < 3; j++) {
                                if (mdl->faces[i].normals[j] == -1) {
                                    normals[j] = mdl->vertices[
                                        mdl->faces[i].vertices[j]
                                    ].normal;
                                }
                                else {
                                    normals[j] = mdl->normals[
                                        mdl->faces[i].normals[j]
                                    ];
                                }
                            }
                            normal = vec3_unit(vec3_add(vec3_add(
                                vec3_mul(normals[0], u),
                                vec3_mul(normals[1], v)),
                                vec3_mul(normals[2], w))
                            );
                        }
                        dot = vec3_dot(rel, normal);
                        mult = calc_mult(
                            ctx->blinn,
                            ctx->brightness,
                            dot,
                            &rel,
                            &normal,
                            &ctx->ambient,
                            &ctx->diffuse,
                            &ctx->specular,
                            ctx->glossiness
                        );
                    }
                    if (z * ZBUF_RES < ctx->zbuf[zbufn]) {
                        ctx->zbuf[zbufn] = (uint32_t) (z * ZBUF_RES);
                        pixels[pixelsn + 0] = SDL_min(mult.x * 255, 255);
                        pixels[pixelsn + 1] = SDL_min(mult.y * 255, 255);
                        pixels[pixelsn + 2] = SDL_min(mult.z * 255, 255);
                    }
                }
                zbufn++;
                pixelsn += 3;
                for (size_t j = 0; j < 3; j++) { xexp[j] -= ydiff[j]; }
            }
            zbufy += ctx->texture->w;
            pixelsy += pitch;
            for (size_t j = 0; j < 3; j++) { yexp[j] += xdiff[j]; }
        }
    }

    SDL_UnlockTexture(ctx->texture);
    SDL_RenderTexture(ctx->renderer, ctx->texture, srcrect, dstrect);

    return true;
}

