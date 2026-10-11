#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coord;

uniform mat4 u_model;      // This can potentially move cube around
uniform mat4 u_view;       // Transforms world into camera's coordinate system
uniform mat4 u_projection; // Allows distant objects to feel smaller

out vec3 v_normal;
out vec3 v_world_pos;
out vec2 tex_coord;

void main()
{
     mat3 normal_matrix = transpose(inverse(mat3(u_model)));
     v_normal = normalize(normal_matrix * a_normal);
     v_world_pos = vec3(u_model * vec4(a_pos, 1.0));
     gl_Position = u_projection * u_view * u_model * vec4(a_pos, 1.0);
     tex_coord = a_tex_coord;
}