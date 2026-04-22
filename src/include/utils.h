#ifndef UTILS_H
#define UTILS_H


typedef struct vec2 {
    double x;
    double y;
} vec2;

double vec2_mag(vec2 vec);
double vec2_mag_sq(vec2 vec);
vec2 vec2_unit(vec2 vec);
void vec2_unit_ip(vec2 *vec);
double vec2_angle(vec2 vec);
double vec2_angle_deg(vec2 vec);
vec2 vec2_add(vec2 addend1, vec2 addend2);
void vec2_add_ip(vec2 *addend1, vec2 addend2);
vec2 vec2_sub(vec2 minuend, vec2 subtrahend);
void vec2_sub_ip(vec2 *minuend, vec2 subtrahend);
vec2 vec2_mul(vec2 multiplicand, double multiplier);
void vec2_mul_ip(vec2 *multiplicand, double multiplier);
vec2 vec2_div(vec2 dividend, double divisor);
void vec2_div_ip(vec2 *dividend, double divisor);
vec2 vec2_scale(vec2 vec, double mag);
void vec2_scale_ip(vec2 *vec, double mag);
vec2 vec2_lerp(vec2 term1, vec2 term2, double t);
void vec2_lerp_ip(vec2 *term1, vec2 term2, double t);
vec2 vec2_mov(vec2 term1, vec2 term2, double disp);
void vec2_mov_ip(vec2 *term1, vec2 term2, double disp);
vec2 vec2_rot(vec2 vec, double angle);
void vec2_rot_ip(vec2 *vec, double angle);
vec2 vec2_rot_deg(vec2 vec, double angle);
void vec2_rot_deg_ip(vec2 *vec, double angle);
double vec2_dot(vec2 term1, vec2 term2);
double vec2_cross(vec2 term1, vec2 term2);
double vec2_dist(vec2 term1, vec2 term2);
double vec2_dist_sq(vec2 term1, vec2 term2);
double vec2_angle_to(vec2 term1, vec2 term2);

double hypot(double x, double y);

#endif

