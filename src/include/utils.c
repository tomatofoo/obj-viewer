#include "SDL3/SDL.h"

#include "utils.h"


double vec2_mag(vec2 vec) {
    return hypot(vec.x, vec.y);
}

double vec2_mag_sq(vec2 vec) {
    return vec2_dot(vec, vec);
}

double vec2_angle(vec2 vec) {
    return SDL_atan2(vec.y, vec.x);
}

double vec2_angle_deg(vec2 vec) {
    return vec2_angle(vec) * 180.0 / SDL_PI_D;
}

vec2 vec2_unit(vec2 vec) {
    double mag = vec2_mag(vec);
    return (vec2) {vec.x / mag, vec.y / mag};
}

void vec2_unit_ip(vec2 *vec) {
    double mag = vec2_mag(*vec);
    vec->x /= mag;
    vec->y /= mag;
}

vec2 vec2_proj(vec2 term1, vec2 term2) {
    return vec2_mul(term2, vec2_dot(term1, term2) / vec2_dot(term2, term2));
}

void vec2_proj_ip(vec2 *term1, vec2 term2) {
    double mult = vec2_dot(*term1, term2) / vec2_dot(term2, term2);
    term1->x *= mult;
    term1->y *= mult;
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
    return (vec2) {
        term1.x + (term2.x - term1.x) * t,
        term1.y + (term2.y - term1.y) * t,
    };
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
    return (vec2) {
        vec.x * cosine - vec.y * sine,
        vec.x * sine + vec.y * cosine,
    };
}

void vec2_rot_ip(vec2 *vec, double angle) {
    double cosine = SDL_cos(angle);
    double sine = SDL_sin(angle);
    double x = vec->x;
    double y = vec->y;
    vec->x = x * cosine - y * sine;
    vec->y = x * sine + y * cosine;
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
    return SDL_acos(
        vec2_dot(term1, term2) / (vec2_mag(term1) * vec2_mag(term2))
    );
}


double vec3_mag(vec3 vec) {
    return SDL_sqrt(vec3_mag_sq(vec));
}

double vec3_mag_sq(vec3 vec) {
    return vec3_dot(vec, vec);
}

double vec3_angle_x(vec3 vec) {
    return SDL_atan2(vec.y, vec.z);
}

double vec3_angle_x_deg(vec3 vec) {
    return vec3_angle_x(vec) * 180.0 / SDL_PI_D;
}

double vec3_angle_y(vec3 vec) {
    return SDL_atan2(vec.z, vec.x);
}

double vec3_angle_y_deg(vec3 vec) {
    return vec3_angle_y(vec) * 180.0 / SDL_PI_D;
}

double vec3_angle_z(vec3 vec) {
    return SDL_atan2(vec.y, vec.x);
}

double vec3_angle_z_deg(vec3 vec) {
    return vec3_angle_z(vec) * 180.0 / SDL_PI_D;
}

vec3 vec3_unit(vec3 vec) {
    double mag = vec3_mag(vec);
    return (vec3) {vec.x / mag, vec.y / mag, vec.z / mag};
}

void vec3_unit_ip(vec3 *vec) {
    double mag = vec3_mag(*vec);
    vec->x /= mag;
    vec->y /= mag;
    vec->z /= mag;
}

vec3 vec3_proj(vec3 term1, vec3 term2) {
    return vec3_mul(term2, vec3_dot(term1, term2) / vec3_dot(term2, term2));
}

void vec3_proj_ip(vec3 *term1, vec3 term2) {
    double mult = vec3_dot(*term1, term2) / vec3_dot(term2, term2);
    term1->x *= mult;
    term1->y *= mult;
    term1->z *= mult;
}

vec3 vec3_add(vec3 addend1, vec3 addend2) {
    return (vec3) {
        addend1.x + addend2.x, addend1.y + addend2.y, addend1.z + addend2.z,
    };
}

void vec3_add_ip(vec3 *addend1, vec3 addend2) {
    addend1->x += addend2.x;
    addend1->y += addend2.y;
    addend1->z += addend2.z;
}

vec3 vec3_sub(vec3 minuend, vec3 subtrahend) {
    return (vec3) {
        minuend.x - subtrahend.x,
        minuend.y - subtrahend.y,
        minuend.z - subtrahend.z,
    };
}

void vec3_sub_ip(vec3 *minuend, vec3 subtrahend) {
    minuend->x -= subtrahend.x;
    minuend->y -= subtrahend.y;
    minuend->z -= subtrahend.z;
}

vec3 vec3_mul(vec3 multiplicand, double multiplier) {
    return (vec3) {
        multiplicand.x * multiplier,
        multiplicand.y * multiplier,
        multiplicand.z * multiplier,
    };
}

void vec3_mul_ip(vec3 *multiplicand, double multiplier) {
    multiplicand->x *= multiplier;
    multiplicand->y *= multiplier;
    multiplicand->z *= multiplier;
}

vec3 vec3_div(vec3 dividend, double divisor) {
    return (vec3) {
        dividend.x / divisor, dividend.y / divisor, dividend.z / divisor,
    };
}

void vec3_div_ip(vec3 *dividend, double divisor) {
    dividend->x /= divisor;
    dividend->y /= divisor;
    dividend->z /= divisor;
}

vec3 vec3_scale(vec3 vec, double mag) {
    double mult = mag / vec3_mag(vec);
    return (vec3) {vec.x * mult, vec.y * mult, vec.z * mult};
}

void vec3_scale_ip(vec3 *vec, double mag) {
    double mult = mag / vec3_mag(*vec);
    vec->x *= mult;
    vec->y *= mult;
    vec->z *= mult;
}

vec3 vec3_lerp(vec3 term1, vec3 term2, double t) {
    return (vec3) {
        term1.x + (term2.x - term1.x) * t,
        term1.y + (term2.y - term1.y) * t,
        term1.z + (term2.z - term1.z) * t,
    };
}

void vec3_lerp_ip(vec3 *term1, vec3 term2, double t) {
    term1->x += (term2.x - term1->x) * t;
    term1->y += (term2.y - term1->y) * t;
    term1->z += (term2.z - term1->z) * t;
}

vec3 vec3_mov(vec3 term1, vec3 term2, double disp) {
    double t = disp / vec3_dist(term1, term2);
    return vec3_lerp(term1, term2, t);
}

void vec3_mov_ip(vec3 *term1, vec3 term2, double disp) {
    double t = disp / vec3_dist(*term1, term2);
    vec3_lerp_ip(term1, term2, t);
}

vec3 vec3_rot_x(vec3 vec, double angle) {
    double cosine = SDL_cos(angle);
    double sine = SDL_sin(angle);
    return (vec3) {
        vec.x, vec.y * cosine - vec.z * sine, vec.y * sine + vec.z * cosine,
    };
}

void vec3_rot_x_ip(vec3 *vec, double angle) {
    double cosine = SDL_cos(angle);
    double sine = SDL_sin(angle);
    double y = vec->y;
    double z = vec->z;
    vec->y = y * cosine - z * sine;
    vec->z = y * sine + z * cosine;
}

vec3 vec3_rot_x_deg(vec3 vec, double angle) {
    return vec3_rot_x(vec, angle / 180.0 * SDL_PI_D);
}

void vec3_rot_x_deg_ip(vec3 *vec, double angle) {
    vec3_rot_x_ip(vec, angle / 180.0 * SDL_PI_D);
}

vec3 vec3_rot_y(vec3 vec, double angle) {
    double cosine = SDL_cos(angle);
    double sine = SDL_sin(angle);
    return (vec3) {
        vec.x * cosine + vec.z * sine, vec.y, -vec.x * sine + vec.z * cosine,
    };
}

void vec3_rot_y_ip(vec3 *vec, double angle) {
    double cosine = SDL_cos(angle);
    double sine = SDL_sin(angle);
    double x = vec->x;
    double z = vec->z;
    vec->x = x * cosine + z * sine;
    vec->z = -x * sine + z * cosine;
}

vec3 vec3_rot_y_deg(vec3 vec, double angle) {
    return vec3_rot_y(vec, angle / 180.0 * SDL_PI_D);
}

void vec3_rot_y_deg_ip(vec3 *vec, double angle) {
    vec3_rot_y_ip(vec, angle / 180.0 * SDL_PI_D);
}

vec3 vec3_rot_z(vec3 vec, double angle) {
    double cosine = SDL_cos(angle);
    double sine = SDL_sin(angle);
    return (vec3) {
        vec.x * cosine - vec.y * sine, vec.x * sine + vec.y * cosine, vec.z,
    };
}

void vec3_rot_z_ip(vec3 *vec, double angle) {
    double cosine = SDL_cos(angle);
    double sine = SDL_sin(angle);
    double x = vec->x;
    double y = vec->y;
    vec->x = x * cosine - y * sine;
    vec->y = x * sine + y * cosine;
}

vec3 vec3_rot_z_deg(vec3 vec, double angle) {
    return vec3_rot_z(vec, angle / 180.0 * SDL_PI_D);
}

void vec3_rot_z_deg_ip(vec3 *vec, double angle) {
    vec3_rot_z_ip(vec, angle / 180.0 * SDL_PI_D);
}

vec3 vec3_cross(vec3 term1, vec3 term2) {
    return (vec3) {
        term1.y * term2.z - term1.z * term2.y,
        term1.z * term2.x - term1.x * term2.z,
        term1.x * term2.y - term1.y * term2.x,
    };
}

void vec3_cross_dest(vec3 term1, vec3 term2, vec3 *dest) {
    dest->x = term1.y * term2.z - term1.z * term2.y;
    dest->y = term1.z * term2.x - term1.x * term2.z;
    dest->z = term1.x * term2.y - term1.y * term2.x;
}

double vec3_dot(vec3 term1, vec3 term2) {
    return term1.x * term2.x + term1.y * term2.y + term1.z * term2.z;
}

double vec3_dist(vec3 term1, vec3 term2) {
    return SDL_sqrt(vec3_dist_sq(term1, term2));
}

double vec3_dist_sq(vec3 term1, vec3 term2) {
    double x = term2.x - term1.x;
    double y = term2.y - term1.y;
    double z = term2.z - term1.z;
    return x * x + y * y + z * z;
}

double vec3_angle_to(vec3 term1, vec3 term2) {
    return SDL_acos(
        vec3_dot(term1, term2) / (vec3_mag(term1) * vec3_mag(term2))
    );
}


double hypot(double x, double y) {
    return SDL_sqrt(x * x + y * y);
}

bool inrange(double x, double l, double h, bool incl, bool inch) {
    if (incl ? x < l : x <= l) { return false; }
    return inch ? x <= h : x < h;
}

const char *filename_ext(const char *filename) {
    while (*filename) {
        if (*filename == '.' && filename[1] != '\0') { return filename + 1; }
        filename++;
    }
    return NULL;
}

