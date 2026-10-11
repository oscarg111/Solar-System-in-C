#ifndef CUBE_H
#define CUBE_H

#include <stdbool.h>
#include <glad/glad.h>
#include "math/math.h"

// Shared gpu mesh resource for drawing cubes
typedef struct
{
    unsigned int vao;
    unsigned int vbo;
} CubeMesh;

// Individual cube instance transformation state
typedef struct
{
    Vec3 position;
    Vec3 scale;
    Vec3 rotation;
} Cube;

CubeMesh cube_mesh_create(void); // init vao vbo
void cube_mesh_destroy(CubeMesh *mesh);
Cube cube_create(Vec3 position, Vec3 scale, Vec3 rotation);            // Create single cube instance
Mat4 cube_get_model_matrix(const Cube *cube);                          // get model transform matrix for specific cube
void cube_draw(const CubeMesh *mesh, const Cube *cube, int loc_model); // render cube instance using shader and mesh

#endif