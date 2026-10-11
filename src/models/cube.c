#include "cube.h"

static const float CUBE_VERTICES[] = {
    // Front face
    -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,

    // Back face
    -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
    1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
    1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
    -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
    -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
    1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f,

    // Left face
    -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,

    // Right face
    1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f,

    // Top face
    -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

    // Bottom face
    -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
    1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
    -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
    -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,
    1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f};

CubeMesh cube_mesh_create(void)
{
    CubeMesh mesh = {0};

    glGenVertexArrays(1, &mesh.vao); // reserves IDs for Vertex Array Objects in open gl
    glGenBuffers(1, &mesh.vbo);      // Generates unique IDs that OpenGL uses to identify buffer objects on GPU

    glBindVertexArray(mesh.vao); // Makes specific VAO active in open gl

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);                                             // connects specific open gl buffer object to active binding target
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES), CUBE_VERTICES, GL_STATIC_DRAW); // Allocates memory space for vertexes

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float))); // Tells opengl how to read vertex data from bound buffer and map to specific attrib in vertex shader
    glEnableVertexAttribArray(1);                                                                    // enables vertex attribute array at a specific index so opengl can use data during rendering

    glBindVertexArray(0); // Makes specific VAO active in open gls global state

    return mesh;
}

void cube_mesh_destroy(CubeMesh *mesh)
{
    if (mesh->vbo)
        glDeleteBuffers(1, &mesh->vbo);
    if (mesh->vao)
        glDeleteVertexArrays(1, &mesh->vao);
    mesh->vbo = 0;
    mesh->vao = 0;
}

Cube cube_create(Vec3 position, Vec3 scale, Vec3 rotation)
{
    Cube cube;

    cube.position = position;
    cube.scale = scale;
    cube.rotation = rotation;

    return cube;
}

Mat4 cube_get_model_matrix(const Cube *cube)
{
    Mat4 translation = mat4_translate(cube->position);
    Mat4 scale = mat4_scale(cube->scale);

    Mat4 rot_x = mat4_rotate_x(cube->rotation.x);
    Mat4 rot_y = mat4_rotate_y(cube->rotation.y);
    Mat4 rot_z = mat4_rotate_z(cube->rotation.z);

    // combine rot
    Mat4 rotation = mat4_multiply(rot_z, mat4_multiply(rot_y, rot_x));

    // Trans * rot * scale
    Mat4 model = mat4_multiply(rotation, scale);
    model = mat4_multiply(translation, model);

    return model;
}

void cube_draw(const CubeMesh *mesh, const Cube *cube, int loc_model)
{
    Mat4 model = cube_get_model_matrix(cube);
    glUniformMatrix4fv(loc_model, 1, GL_TRUE, model.m); // send matrix to vertex shader

    glBindVertexArray(mesh->vao);
    glDrawArrays(GL_TRIANGLES, 0, 36); // actually draw out shape
    glBindVertexArray(0);
}