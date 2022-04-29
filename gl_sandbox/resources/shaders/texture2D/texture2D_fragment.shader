#version 410

uniform sampler2D tex;

in vec3 v_normal;
in vec2 v_tex_coord;

uniform vec3 u_light;

out vec4 colour;

void main()
{
	colour = texture(tex, v_tex_coord) * dot(v_normal, u_light);
}