#ifndef SPHERE_H
#define SPHERE_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <glad/glad.h>
#include "math/math.h"

typedef struct
{
    unsigned int vao;
    unsigned int vbo;
} SphereMesh;

typedef struct
{
    Vec3 position;
    Vec3 rotation;
    float radius;
    unsigned int texture;
} Sphere;

float *get_sphere_vertices(void);
SphereMesh sphere_mesh_create(void);
void sphere_mesh_destroy(SphereMesh *mesh);
Sphere sphere_create(Vec3 position, Vec3 rotation, float radius, char *image_file_name);
Mat4 sphere_get_model_matrix(const Sphere *sphere);
void sphere_draw(const SphereMesh *mesh, const Sphere *sphere, int loc_model);

#endif