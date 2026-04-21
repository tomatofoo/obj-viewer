#ifndef UTILS_H
#define UTILS_H

typedef struct vec2 {
    double x;
    double y;
} vec2;

double vec2_angle(vec2 vec);
double vec2_angle_deg(vec2 vec);
vec2 vec2_add(vec2 addend1, vec2 addend2);
vec2 vec2_sub(vec2 minuend, vec2 subtrahend);
vec2 vec2_mul(vec2 multiplicand, double multiplier);
double vec2_dot(vec2 term1, vec2 term2);
double vec2_cross(vec2 term1, vec2 term2);

#endif

