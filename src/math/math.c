#include "./math.h"
#include <math.h>

// function for creating a vector
Vec3 vec3_create(float x, float y, float z)
{
    Vec3 result;

    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}

// add two vectors
Vec3 vec3_add(Vec3 a, Vec3 b)
{
    return vec3_create(
        a.x + b.x,
        a.y + b.y,
        a.z + b.z);
}

// vector subtraction
Vec3 vec3_sub(Vec3 a, Vec3 b)
{
    return vec3_create(
        a.x - b.x,
        a.y - b.y,
        a.z - b.z);
}

// scale a vector
Vec3 vec3_scale(Vec3 v, float scalar)
{
    return vec3_create(
        v.x * scalar,
        v.y * scalar,
        v.z * scalar);
}

// get the length of a vector
float vec3_length(Vec3 v)
{
    return sqrtf(
        v.x * v.x +
        v.y * v.y +
        v.z * v.z);
}

// Normalize vector (same direction but of length one)
Vec3 vec3_normalize(Vec3 v)
{
    float length = vec3_length(v);

    if (length == 0.0f)
        return vec3_create(0.0f, 0.0f, 0.0f);

    return vec3_create(
        v.x / length,
        v.y / length,
        v.z / length);
}

// Dot product of vectors A · B = AxBx + AyBy + AzBz
float vec3_dot(Vec3 a, Vec3 b)
{
    return a.x * b.x +
           a.y * b.y +
           a.z * b.z;
}

// Cross product produces a vector perpendicular to both vectors
// A × B =
// (
//     AyBz - AzBy,
//     AzBx - AxBz,
//     AxBy - AyBx
// )
Vec3 vec3_cross(Vec3 a, Vec3 b)
{
    return vec3_create(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x);
}

// 4x4 matrix functions
Mat4 mat4_identity()
{
    Mat4 identity = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

    return identity;
}

// multiply two matrix
Mat4 mat4_multiply(Mat4 a, Mat4 b)
{
    Mat4 result = {0};

    for (int i = 0; i < 4; i++) // traverse rows
    {
        for (int j = 0; j < 4; j++) // traverse columns
        {
            for (int k = 0; k < 4; k++) // addition of multiplication for row/col
            {
                result.m[i * 4 + j] +=
                    a.m[i * 4 + k] *
                    b.m[k * 4 + j];
            }
        }
    }

    return result;
}

// mat4_perspective
Mat4 mat4_perspective(
    float fov_radians,
    float aspect,
    float near,
    float far)
{
    Mat4 result = {0};

    float f = 1.0f / tanf(fov_radians / 2.0f);

    result.m[0] = f / aspect;
    result.m[5] = f;

    result.m[10] = (far + near) / (near - far);
    result.m[11] = (2.0f * far * near) / (near - far);

    result.m[14] = -1.0f;

    return result;
}

// mat4_look_at
Mat4 mat4_look_at(Vec3 eye, Vec3 target, Vec3 world_up)
{
    Vec3 forward = vec3_normalize(vec3_sub(eye, target));
    Vec3 right = vec3_normalize(vec3_cross(world_up, forward));
    Vec3 up = vec3_cross(forward, right);

    Mat4 result = {0};

    result.m[0] = right.x;
    result.m[1] = right.y;
    result.m[2] = right.z;
    result.m[3] = -vec3_dot(right, eye);

    result.m[4] = up.x;
    result.m[5] = up.y;
    result.m[6] = up.z;
    result.m[7] = -vec3_dot(up, eye);

    result.m[8] = forward.x;
    result.m[9] = forward.y;
    result.m[10] = forward.z;
    result.m[11] = -vec3_dot(forward, eye);

    result.m[12] = 0.0f;
    result.m[13] = 0.0f;
    result.m[14] = 0.0f;
    result.m[15] = 1.0f;

    return result;
}

// mat4 translate
Mat4 mat4_translate(Vec3 t)
{
    // start from identity matrix
    Mat4 result = mat4_identity();

    // translate x goes in slot 3, y in slot 7, z in slot 11
    result.m[3] = t.x;
    result.m[7] = t.y;
    result.m[11] = t.z;

    return result;
}

// mat4 scale
Mat4 mat4_scale(Vec3 s)
{
    // start from identity (or zero) matrix
    Mat4 result = mat4_identity();

    // insert scale x in 0, y in 5, z in 10
    result.m[0] = s.x;
    result.m[5] = s.y;
    result.m[10] = s.z;

    return result;
}

// rotation functions
Mat4 mat4_rotate_x(float rad)
{
    Mat4 result = {0};

    result.m[0] = 1;
    result.m[5] = cosf(rad);
    result.m[6] = -sinf(rad);
    result.m[9] = sinf(rad);
    result.m[10] = cosf(rad);
    result.m[15] = 1;

    return result;
}

Mat4 mat4_rotate_y(float rad)
{
    Mat4 result = {0};

    result.m[0] = cosf(rad);
    result.m[2] = sinf(rad);
    result.m[5] = 1;
    result.m[8] = -sinf(rad);
    result.m[10] = cosf(rad);
    result.m[15] = 1;

    return result;
}

Mat4 mat4_rotate_z(float rad)
{
    Mat4 result = {0};

    result.m[0] = cosf(rad);
    result.m[1] = -sinf(rad);
    result.m[4] = sinf(rad);
    result.m[5] = cosf(rad);
    result.m[10] = 1;
    result.m[15] = 1;

    return result;
}
