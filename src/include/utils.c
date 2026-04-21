#include <math.h>

#include "utils.h"


double vec2_angle(vec2 vec) {
    return atan2(vec.y, vec.x);
}

double vec2_angle_deg(vec2 vec) {
    return atan2(vec.y, vec.x) * 180.0 / M_PI;
}

vec2 vec2_add(vec2 addend1, vec2 addend2) {
    return (vec2) {addend1.x + addend2.x, addend1.y + addend2.y};
}

vec2 vec2_sub(vec2 minuend, vec2 subtrahend) {
    return (vec2) {minuend.x - subtrahend.x, minuend.y - subtrahend.y};
}

vec2 vec2_mul(vec2 multiplicand, double multiplier) {
    return (vec2) {multiplicand.x * multiplier, multiplicand.y * multiplier};
}

vec2 vec2_div(vec2 dividend, double divisor) {
    return (vec2) {dividend.x / divisor, dividend.y / divisor};
}

double vec2_dot(vec2 term1, vec2 term2) {
    return term1.x * term2.x + term1.y * term2.y;
}

double vec2_cross(vec2 term1, vec2 term2) {
    return term1.x * term2.y - term1.y * term2.x;
}

