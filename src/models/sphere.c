#include "sphere.h"
#include "stb_image.h"

#define M_PI 3.14159265358979323846264338327950288

const int SECTOR_COUNT = 50; // left to right sections
const int STACK_COUNT = 50;  // top to bottom sections

float *get_sphere_vertices(void)
{
    // x = (r * cos(phi)) * cos(theta)
    // y = (r * cos(phi)) * sin(theta)
    // z = r * sin(phi)

    // theta = 2 * pi * (sectorStep / sectorCount)
    // phi = pi / 2 - pi * (stackStep / stackCount)
    // In this case, theta is the left to right angle
    // and phi is top to bottom angle, going from 0 -> 360 for
    // theta and 90 -> -90 for phi

    float *result = malloc((SECTOR_COUNT * STACK_COUNT * 6 * 6 * 2 * 2) * sizeof(float)); // everything twice because each section contains two triangles
    int idx = 0;

    // float x, y, z, xy;
    // float nx, ny, nz, length_inv = 1.0f;

    float sector_step = 2 * M_PI / SECTOR_COUNT;
    float stack_step = M_PI / STACK_COUNT;
    // float sector_angle, stack_angle;

    // go through sectors
    for (int stack = 0; stack < STACK_COUNT; stack++)
    {
        // calculate stack angle, xy (r * cos(phi)) and z
        float phi0 = (M_PI / 2) - (stack * stack_step);
        float phi1 = (M_PI / 2) - (stack + 1) * stack_step;

        // go through stacks
        for (int sector = 0; sector < SECTOR_COUNT; sector++)
        {
            // calculate sector angle, x, and y, as well as push those to result array
            float theta0 = sector * sector_step;
            float theta1 = (sector + 1) * sector_step;

            // 4 corners of this quad, on a unit sphere (y-up, to match the rest of the project)
            Vec3 p00 = vec3_create(cosf(phi0) * cosf(theta0), sinf(phi0), cosf(phi0) * sinf(theta0));
            Vec3 p01 = vec3_create(cosf(phi0) * cosf(theta1), sinf(phi0), cosf(phi0) * sinf(theta1));
            Vec3 p10 = vec3_create(cosf(phi1) * cosf(theta0), sinf(phi1), cosf(phi1) * sinf(theta0));
            Vec3 p11 = vec3_create(cosf(phi1) * cosf(theta1), sinf(phi1), cosf(phi1) * sinf(theta1));

            Vec3 corners[6] = {p00, p10, p01, p01, p10, p11}; // 2 triangles covering the quad

            float s = (float)sector / SECTOR_COUNT;
            float t = (float)stack / STACK_COUNT;

            for (int v = 0; v < 6; v++)
            {
                result[idx++] = corners[v].x;
                result[idx++] = corners[v].y;
                result[idx++] = corners[v].z;
                // normal = position, since every corner already sits on the unit sphere
                result[idx++] = corners[v].x;
                result[idx++] = corners[v].y;
                result[idx++] = corners[v].z;
                // set tex coords

                result[idx++] = s;
                result[idx++] = t;
            }
        }
    }

    return result;
}

SphereMesh sphere_mesh_create(void)
{
    SphereMesh mesh = {0};

    float *sphere_vertices = get_sphere_vertices();

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, SECTOR_COUNT * STACK_COUNT * 6 * 6 * 2 * 2 * sizeof(float), sphere_vertices, GL_STATIC_DRAW);
    free(sphere_vertices);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    return mesh;
}

void sphere_mesh_destroy(SphereMesh *mesh)
{
    if (mesh->vbo)
        glDeleteBuffers(1, &mesh->vbo);
    if (mesh->vao)
        glDeleteVertexArrays(1, &mesh->vao);
    mesh->vbo = 0;
    mesh->vao = 0;
}

Sphere sphere_create(Vec3 position, Vec3 rotation, float radius, char *image_file_name)
{
    Sphere sphere;

    sphere.position = position;
    sphere.radius = radius;
    sphere.rotation = rotation;

    // load texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(image_file_name, &width, &height, &nrChannels, 0);
    if (data)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        fprintf(stderr, "Error Loading texture\n");
    }

    sphere.texture = texture;

    stbi_image_free(data);

    return sphere;
}

Mat4 sphere_get_model_matrix(const Sphere *sphere)
{
    Mat4 translation = mat4_translate(sphere->position);
    // Mat4 scale = mat4_scale(sphere->scale);

    Mat4 rot_x = mat4_rotate_x(sphere->rotation.x);
    Mat4 rot_y = mat4_rotate_y(sphere->rotation.y);
    Mat4 rot_z = mat4_rotate_z(sphere->rotation.z);

    // combine rot
    Mat4 rotation = mat4_multiply(rot_z, mat4_multiply(rot_y, rot_x));

    // Trans * rot * scale
    // Mat4 model = mat4_multiply(rotation, scale);
    Mat4 scale = mat4_scale(vec3_create(sphere->radius, sphere->radius, sphere->radius));
    Mat4 model = mat4_multiply(rotation, scale);
    model = mat4_multiply(translation, model);

    return model;
}

void sphere_draw(const SphereMesh *mesh, const Sphere *sphere, int loc_model)
{
    Mat4 model = sphere_get_model_matrix(sphere);
    glUniformMatrix4fv(loc_model, 1, GL_TRUE, model.m); // send matrix to vertex shader

    glBindTexture(GL_TEXTURE_2D, sphere->texture);
    glBindVertexArray(mesh->vao);
    glDrawArrays(GL_TRIANGLES, 0, SECTOR_COUNT * STACK_COUNT * 6); // actually draw out shape
    glBindVertexArray(0);
}