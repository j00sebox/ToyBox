#version 410

uniform sampler2D diffuse_t;
uniform sampler2D specular_t;

in vec3 v_position;
in vec3 v_normal;
in vec2 v_tex_coord;

uniform int u_use_pl;
uniform vec3 u_directional_light;
uniform vec3 u_cam_pos;

uniform bool u_use_colour;
uniform vec4 u_flat_colour;

// point light stuff
uniform vec3 u_pl_pos;
uniform vec4 u_pl_col;
uniform float u_pl_rad;
uniform float u_pl_range;

int glossiness = 4;
// ambient light
float ambient = 0.2f;
const float pi = 3.14159265358f;

out vec4 colour;

float point_light()
{
	vec3 light_vec = u_pl_pos - v_position;
	float distance = length(light_vec);

	if (distance > u_pl_range)
	{
		return 0.f;
	}
	
    return 2.f / (distance * distance + u_pl_rad * u_pl_rad + distance * sqrt(distance * distance + u_pl_rad * u_pl_rad));
}

vec4 lambertian()
{
	vec4 base_colour;

	if (u_use_colour)
	{
		base_colour = u_flat_colour;
	}
	else
	{
		base_colour = texture(diffuse_t, v_tex_coord);
	}

	return base_colour / pi;
}

void main()
{
	if (u_use_pl == 1)
	{
		colour = point_light() * lambertian();
	}
	else
	{
		colour = texture(diffuse_t, v_tex_coord) * ambient;
	}
}