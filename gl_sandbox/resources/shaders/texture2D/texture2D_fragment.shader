#version 410

uniform sampler2D diffuse_t;
uniform sampler2D specular_t;

in vec3 v_position;
in vec3 v_normal;
in vec2 v_tex_coord;

uniform int u_use_pl;
uniform vec3 u_directional_light;
uniform vec3 u_cam_pos;

uniform int u_use_colour;
uniform vec4 u_flat_colour;

// point light stuff
uniform vec3 u_pl_pos;
uniform vec4 u_pl_col;
uniform float u_pl_rad;
uniform float u_pl_range;

int glossiness = 4;
// ambient light
float ambient = 0.2f;

out vec4 colour;

vec4 point_light()
{
	vec4 base_colour;

	if (u_use_colour == 1)
	{
		base_colour = u_flat_colour;
	}
	else
	{
		base_colour = texture(diffuse_t, v_tex_coord);
	}

	vec3 light_vec = u_pl_pos - v_position;
	float distance = length(light_vec);

	if (distance > u_pl_range)
	{
		return base_colour * ambient;
	}
	
    float attenuation = 2.f / (distance * distance + u_pl_rad * u_pl_rad + distance * sqrt(distance * distance + u_pl_rad * u_pl_rad));

	// diffuse 
	vec3 normal = normalize(v_normal);
	vec3 direction = normalize(light_vec);
	float diffuse = abs(dot(normal, direction));

	// specular
	float ks = 1.f;
	vec3 viewing_dir = normalize(u_cam_pos - u_pl_pos);
	vec3 perfect_reflect = reflect(-direction, normal);
	float specular = pow(max(dot(perfect_reflect, viewing_dir), 0.0), glossiness);

	return  base_colour * (diffuse * attenuation * u_pl_col + ambient); // +texture(specular_t, v_tex_coord).r * specular * ks * u_pl_col;
}

void main()
{
	if (u_use_pl == 1)
	{
		colour = point_light();
	}
	else
	{
		colour = texture(diffuse_t, v_tex_coord) * ambient;
	}
}