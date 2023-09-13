#version 420

in vec3 v_position;
in vec3 v_normal;
in vec2 v_tex_coord;
in mat3 v_model;

uniform sampler2D scene_t;

out vec4 colour;

void main()
{
    colour = texture(scene_t, v_tex_coord);
}