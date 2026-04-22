#include "SDL3/SDL.h"

#include "utils.h"


double vec2_mag(vec2 vec) {
    return hypot(vec.x, vec.y);
}

double vec2_mag_sq(vec2 vec) {
    return vec.x * vec.x + vec.y * vec.y;
}

vec2 vec2_normalize(vec2 vec) {
    double mag = vec2_mag(vec);
    return (vec2) {vec.x / mag, vec.y / mag};
}

void vec2_normalize_ip(vec2 *vec) {
    double mag = vec2_mag(*vec);
    vec->x /= mag;
    vec->y /= mag;
}

double vec2_angle(vec2 vec) {
    return SDL_atan2(vec.y, vec.x);
}

double vec2_angle_deg(vec2 vec) {
    return SDL_atan2(vec.y, vec.x) * 180.0 / SDL_PI_D;
}

vec2 vec2_add(vec2 addend1, vec2 addend2) {
    return (vec2) {addend1.x + addend2.x, addend1.y + addend2.y};
}

void vec2_add_ip(vec2 *addend1, vec2 addend2) {
    addend1->x += addend2.x;
    addend1->y += addend2.y;
}

vec2 vec2_sub(vec2 minuend, vec2 subtrahend) {
    return (vec2) {minuend.x - subtrahend.x, minuend.y - subtrahend.y};
}

void vec2_sub_ip(vec2 *minuend, vec2 subtrahend) {
    minuend->x -= subtrahend.x;
    minuend->y -= subtrahend.y;
}

vec2 vec2_mul(vec2 multiplicand, double multiplier) {
    return (vec2) {multiplicand.x * multiplier, multiplicand.y * multiplier};
}

void vec2_mul_ip(vec2 *multiplicand, double multiplier) {
    multiplicand->x *= multiplier;
    multiplicand->y *= multiplier;
}

vec2 vec2_div(vec2 dividend, double divisor) {
    return (vec2) {dividend.x / divisor, dividend.y / divisor};
}

void vec2_div_ip(vec2 *dividend, double divisor) {
    dividend->x /= divisor;
    dividend->y /= divisor;
}

vec2 vec2_scale(vec2 vec, double mag) {
    double mult = mag / vec2_mag(vec);
    return (vec2) {vec.x * mult, vec.y * mult};
}

void vec2_scale_ip(vec2 *vec, double mag) {
    double mult = mag / vec2_mag(*vec);
    vec->x *= mult;
    vec->y *= mult;
}

vec2 vec2_lerp(vec2 term1, vec2 term2, double t) {
    return (vec2) {term1.x + (term2.x - term1.x) * t, term1.y + (term2.y - term1.y) * t};
}

void vec2_lerp_ip(vec2 *term1, vec2 term2, double t) {
    term1->x += (term2.x - term1->x) * t;
    term1->y += (term2.y - term1->y) * t;
}

vec2 vec2_mov(vec2 term1, vec2 term2, double disp) {
    double t = disp / vec2_dist(term1, term2);
    return vec2_lerp(term1, term2, t);
}

void vec2_mov_ip(vec2 *term1, vec2 term2, double disp) {
    double t = disp / vec2_dist(*term1, term2);
    vec2_lerp_ip(term1, term2, t);
}

vec2 vec2_rot(vec2 vec, double angle) {
    double cosine = SDL_cos(angle);
    double sine = SDL_sin(angle);
    return (vec2) {vec.x * cosine - vec.y * sine, vec.x * sine + vec.y * cosine};
}

void vec2_rot_ip(vec2 *vec, double angle) {
    double cosine = SDL_cos(angle);
    double sine = SDL_sin(angle);
    vec->x = vec->x * cosine - vec->y * sine;
    vec->y = vec->x * sine + vec->y * cosine;
}

vec2 vec2_rot_deg(vec2 vec, double angle) {
    return vec2_rot(vec, angle / 180.0 * SDL_PI_D);
}

void vec2_rot_deg_ip(vec2 *vec, double angle) {
    vec2_rot_ip(vec, angle / 180.0 * SDL_PI_D);
}

double vec2_dot(vec2 term1, vec2 term2) {
    return term1.x * term2.x + term1.y * term2.y;
}

double vec2_cross(vec2 term1, vec2 term2) {
    return term1.x * term2.y - term1.y * term2.x;
}

double vec2_dist(vec2 term1, vec2 term2) {
    return hypot(term2.x - term1.x, term2.y - term1.y);
}

double vec2_dist_sq(vec2 term1, vec2 term2) {
    double x = term2.x - term1.x;
    double y = term2.y - term1.y;
    return x * x + y * y;
}

double vec2_angle_to(vec2 term1, vec2 term2) {
    return SDL_acos(vec2_dot(term1, term2) / (vec2_mag(term1) * vec2_mag(term2)));
}


double hypot(double x, double y) {
    return SDL_sqrt(x * x + y * y);
}

