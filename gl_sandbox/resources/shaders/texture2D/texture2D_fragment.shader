#version 410

uniform sampler2D diffuse;
uniform sampler2D specular;

in vec3 v_normal;
in vec2 v_tex_coord;

uniform vec3 u_light;

out vec4 colour;

void main()
{
	colour = texture(diffuse, v_tex_coord) * dot(v_normal, u_light) * 0.5f + texture(specular, v_tex_coord) * dot(v_normal, u_light) * 0.5f;
}