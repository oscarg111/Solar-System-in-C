#version 330 core

in vec3 v_normal;
in vec3 v_world_pos;
out vec4 frag_color;
uniform vec4 u_color;
uniform vec3 u_light_pos;
uniform vec3 u_light_color;
in vec2 tex_coord;

uniform sampler2D tex_value;

void main() {
    vec3 N = normalize(v_normal);
    vec3 L = normalize(u_light_pos - v_world_pos);
    vec3 ambient = 0.1 * u_light_color;
    vec3 diffuse = max(dot(N, L), 0.0) * u_light_color;
    // frag_color = vec4((ambient + diffuse) * u_color.rgb, u_color.a);
    frag_color = texture(tex_value, tex_coord) * vec4((ambient + diffuse) * u_color.rgb, u_color.a);
    // frag_color = vec4(tex_coord, 0.0, 1.0);

}