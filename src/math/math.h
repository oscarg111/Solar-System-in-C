#ifndef MATH_H
#define MATH_H

typedef struct {
    float x;
    float y; 
    float z;
} Vec3;

typedef struct {
    float m[16];
} Mat4;

// Vec 3
Vec3 vec3_create(float x, float y, float z);

Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_sub(Vec3 a, Vec3 b);
Vec3 vec3_scale(Vec3 v, float scalar);

float vec3_length(Vec3 v);
Vec3 vec3_normalize(Vec3 v);

float vec3_dot(Vec3 a, Vec3 b);
Vec3 vec3_cross(Vec3 a, Vec3 b);

// Mat4
Mat4 mat4_identity();
Mat4 mat4_multiply(Mat4 a, Mat4 b);
Mat4 mat4_perspective(float fov_radians, float aspect, float near, float far);
Mat4 mat4_look_at(Vec3 eye, Vec3 target, Vec3 world_up);
Mat4 mat4_translate(Vec3 t);
Mat4 mat4_scale(Vec3 s);
Mat4 mat4_rotate_x(float rad);
Mat4 mat4_rotate_y(float rad);
Mat4 mat4_rotate_z(float rad);

#endif