#ifndef UTILS_H
#define UTILS_H


#define BLACK 0, 0, 0
#define WHITE 255, 255, 255
#define RED 255, 0, 0
#define GREEN 0, 255, 0
#define BLUE 0, 0, 255
#define CYAN 0, 255, 255
#define MAGENTA 255, 0, 255
#define YELLOW 255, 255, 0

#define arr_sizeof(arr) sizeof(arr) / sizeof(arr[0])


typedef struct vec2 {
    double x;
    double y;
} vec2;

typedef struct vec3 {
    double x;
    double y;
    double z;
} vec3;


double vec2_mag(vec2 vec);
double vec2_mag_sq(vec2 vec);
double vec2_angle(vec2 vec);
double vec2_angle_deg(vec2 vec);
vec2 vec2_unit(vec2 vec);
void vec2_unit_ip(vec2 *vec);
vec2 vec2_proj(vec2 term1, vec2 term2);
void vec2_proj_ip(vec2 *term1, vec2 term2);
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

double vec3_mag(vec3 vec);
double vec3_mag_sq(vec3 vec);
double vec3_angle_x(vec3 vec);
double vec3_angle_x_deg(vec3 vec);
double vec3_angle_y(vec3 vec);
double vec3_angle_y_deg(vec3 vec);
double vec3_angle_z(vec3 vec);
double vec3_angle_z_deg(vec3 vec);
vec3 vec3_unit(vec3 vec);
void vec3_unit_ip(vec3 *vec);
vec3 vec3_proj(vec3 term1, vec3 term2);
void vec3_proj_ip(vec3 *term1, vec3 term2);
vec3 vec3_add(vec3 addend1, vec3 addend2);
void vec3_add_ip(vec3 *addend1, vec3 addend2);
vec3 vec3_sub(vec3 minuend, vec3 subtrahend);
void vec3_sub_ip(vec3 *minuend, vec3 subtrahend);
vec3 vec3_mul(vec3 multiplicand, double multiplier);
void vec3_mul_ip(vec3 *multiplicand, double multiplier);
vec3 vec3_div(vec3 dividend, double divisor);
void vec3_div_ip(vec3 *dividend, double divisor);
vec3 vec3_scale(vec3 vec, double mag);
void vec3_scale_ip(vec3 *vec, double mag);
vec3 vec3_lerp(vec3 term1, vec3 term2, double t);
void vec3_lerp_ip(vec3 *term1, vec3 term2, double t);
vec3 vec3_mov(vec3 term1, vec3 term2, double disp);
void vec3_mov_ip(vec3 *term1, vec3 term2, double disp);
vec3 vec3_rot_x(vec3 vec, double angle);
void vec3_rot_x_ip(vec3 *vec, double angle);
vec3 vec3_rot_x_deg(vec3 vec, double angle);
void vec3_rot_x_deg_ip(vec3 *vec, double angle);
vec3 vec3_rot_y(vec3 vec, double angle);
void vec3_rot_y_ip(vec3 *vec, double angle);
vec3 vec3_rot_y_deg(vec3 vec, double angle);
void vec3_rot_y_deg_ip(vec3 *vec, double angle);
vec3 vec3_rot_z(vec3 vec, double angle);
void vec3_rot_z_ip(vec3 *vec, double angle);
vec3 vec3_rot_z_deg(vec3 vec, double angle);
void vec3_rot_z_deg_ip(vec3 *vec, double angle);
vec3 vec3_cross(vec3 term1, vec3 term2);
void vec3_cross_dest(vec3 term1, vec3 term2, vec3 *dest);
double vec3_dot(vec3 term1, vec3 term2);
double vec3_dist(vec3 term1, vec3 term2);
double vec3_dist_sq(vec3 term1, vec3 term2);
double vec3_angle_to(vec3 term1, vec3 term2);

double hypot(double x, double y);
bool inrange(double x, double l, double h, bool incl, bool inch);
const char *filename_ext(const char *filename);

#endif

