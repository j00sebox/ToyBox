#version 410

uniform sampler2D diffuse;
uniform sampler2D specular;

in vec3 v_position;
in vec3 v_normal;
in vec2 v_tex_coord;

//uniform vec3 u_light;

// point light stuff
uniform vec3 u_pl_pos;
uniform vec4 u_pl_col;
uniform float u_pl_rad;

out vec4 colour;

float point_light_attenuation()
{
	vec3 distance_vec = u_pl_pos - v_position;
	float distance = sqrt(distance_vec.x * distance_vec.x + distance_vec.y * distance_vec.y + distance_vec.z * distance_vec.z);
	float divisor = distance * distance + u_pl_rad * u_pl_rad + distance * sqrt(distance * distance + u_pl_rad * u_pl_rad);
	return 2.f / divisor;
}

void main()
{
	colour = texture(diffuse, v_tex_coord) * u_pl_col * point_light_attenuation();
}