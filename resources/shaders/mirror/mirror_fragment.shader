#version 410

in vec3 v_position;
in vec3 v_normal;
in vec2 v_tex_coord;
in mat3 v_model;

uniform sampler2D scene_t;

void main()
{
    color = texture(scene_t, v_tex_coord);
}