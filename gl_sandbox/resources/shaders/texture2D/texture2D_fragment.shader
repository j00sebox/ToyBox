#version 410

uniform sampler2D diffuse_t;
uniform sampler2D specular_t;
uniform sampler2D normal_t;
uniform sampler2D occlusion_t;

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
float roughness = 0.7;
// ambient light
float ambient = 0.2f;
const float pi = 3.14159265358f;

out vec4 colour;

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

// Schlick
float fresnel(vec3 h, vec3 v)
{
	float F0 = 0.1f;

	float h_dot_v = max(dot(h, v), 0.0f);

	return F0 + (1 - F0) * pow((1 - h_dot_v), 5);
}

// Trowbridge-Reitz
float normal_distribution(vec3 h, vec3 n)
{
	float alpha = roughness * roughness;
	float h_dot_n = max(dot(h, n), 0.0f);
	float numerator = alpha * alpha;
	float denom = pi * pow((pow(h_dot_n, 2) * (numerator - 1) + 1), 2);

	return numerator / denom;
}

float G_Schlick(vec3 n, vec3 v)
{
	float n_dot_v = max(dot(n, v), 0.0f);

	return n_dot_v / (n_dot_v * (1 - roughness) + roughness);
}

float G_Smith(vec3 n, vec3 v, vec3 l)
{
	return G_Schlick(n, v) * G_Schlick(n, l);
}

vec4 point_light()
{
	vec3 light_vec = u_pl_pos - v_position;	
	float distance = length(light_vec);
	vec3 l = normalize(light_vec);
	vec3 v = normalize(u_cam_pos - v_position);
	vec3 n = normalize(v_normal);
	vec3 h = normalize(l + v);

	if (distance > u_pl_range)
	{
		return lambertian() * ambient;
	}

	float ks = fresnel(h, v);
	float kd = 1 - ks;

	float D = normal_distribution(h, n);
	float G = G_Smith(n, v, l);

	float v_dot_n = max(dot(v, n), 0.000001f);
	float l_dot_n = max(dot(l, n), 0.000001f);

	// Cook-Torrance
	float specular = (D * G * ks) / (4 * v_dot_n * l_dot_n);

	vec4 brdf = kd * lambertian() + specular;

	float h_dot_n = max(dot(h, n), 0.0f);
	
    float attenuation =  2.f / (distance * distance + u_pl_rad * u_pl_rad + distance * sqrt(distance * distance + u_pl_rad * u_pl_rad));

	return  brdf * attenuation * u_pl_col * h_dot_n;
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