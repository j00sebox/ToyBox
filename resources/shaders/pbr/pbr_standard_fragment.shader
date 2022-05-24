#version 410

/*----------Textures----------*/

uniform sampler2D diffuse_t;
uniform sampler2D specular_t;
uniform sampler2D normal_t;
uniform sampler2D occlusion_t;
uniform vec4 u_emissive_colour;

in vec3 v_position;
in vec3 v_normal;
in vec2 v_tex_coord;
in mat3 v_model;

uniform vec3 u_cam_pos;

uniform bool u_custom;
uniform vec4 u_flat_colour;
uniform float u_metallic;
uniform float u_roughness;

vec4 base_colour;
vec3 normal;
float metallic;
float roughness;
float ao;
vec4 F0 = vec4(vec3(0.04f), 1.0f);

const float pi = 3.14159265358f;

/*----------Lighting----------*/

struct DirectionalLight
{
	bool active;
	vec4 colour;
	vec3 direction;
	float brightness;
};

uniform DirectionalLight directional_light;

#define MAX_POINT_LIGHTS 2

struct PointLight
{
	bool active;
	vec4 colour;
	vec3 position;
	float range;
	float radius;
	float brightness;
};

uniform PointLight point_lights[MAX_POINT_LIGHTS];

out vec4 colour;

// various kernels

float sharper_kernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
);

float blur_kernel[9] = float[](
    1.f / 16.f, 2.f / 16.f, 1.f / 16.f,
    2.f / 16.f, 4.f / 16.f, 2.f / 16.f,
    1.f / 16.f, 2.f / 16.f, 1.f / 16.f  
);

// assuming the kernel is 3x3
vec4 apply_kernel(float[9] kernel, sampler2D tex)
{
	const float offset = 1.0 / 300.0;  

    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );

	vec4 samples[9];
	for(int i = 0; i < 9; i++)
	{
		samples[i] = texture(tex, v_tex_coord + offsets[i]);
	}

	vec4 res_colour;
	for(int i = 0; i < 9; i++)
	{
		res_colour += samples[i] * kernel[i];
	}

	return res_colour;
}

vec4 lambertian()
{
	return base_colour / pi;
}

// Schlick
vec4 fresnel(vec3 h, vec3 v)
{
	float h_dot_v = max(dot(h, v), 0.0f);

	return F0 + (1 - F0) * pow((1 - h_dot_v), 5);
}

// Trowbridge-Reitz
float normal_distribution(vec3 h, vec3 n)
{
	float alpha = roughness * roughness;
	float h_dot_n = max(dot(h, n), 0.0f);
	float alpha_sq = alpha * alpha;
	float denom = pi * pow((pow(h_dot_n, 2) * (alpha_sq - 1) + 1), 2);

	return alpha_sq / denom;
}

float G_Schlick(vec3 n, vec3 v)
{
	float r = (roughness + 1.f);
	float k = (r * r) / 8.f;

	float n_dot_v = max(dot(n, v), 0.0f);

	return n_dot_v / (n_dot_v * (1 - k) + k);
}

float G_Smith(vec3 n, vec3 v, vec3 l)
{
	return G_Schlick(n, v) * G_Schlick(n, l);
}

vec4 point_light(int i)
{
	vec3 light_vec = point_lights[i].position - v_position;
	float distance = length(light_vec);
	vec3 l = normalize(light_vec);
	vec3 v = normalize(u_cam_pos - v_position);
	vec3 n = normalize(v_model * normal);
	vec3 h = normalize(l + v);

	if (distance > point_lights[i].range)
	{
		return vec4(0.f);
	}

	vec4 ks = fresnel(h, v);
	vec4 kd = vec4(1 - vec3(ks), 1.f);
	kd *= (1 - metallic);

	float D = normal_distribution(h, n);
	float G = G_Smith(n, v, l);

	float v_dot_n = max(dot(v, n), 0.000001f);
	float l_dot_n = max(dot(l, n), 0.000001f);

	// Cook-Torrance
	vec4 specular = (ks * D * G) / (4 * v_dot_n * l_dot_n);

	vec4 brdf = kd * lambertian() + specular;

	float h_dot_n = max(dot(h, n), 0.0f);
	
	float attenuation = 1 / (distance * distance);

	return  brdf * attenuation * point_lights[i].colour * h_dot_n * point_lights[i].brightness;
}

vec4 direct_light()
{
	vec3 l = normalize(directional_light.direction);
	vec3 v = normalize(u_cam_pos - v_position);
	vec3 n = normalize(v_model * normal); // TODO: will need to change once non uniform scaling is implemented
	vec3 h = normalize(l + v);

	vec4 ks = fresnel(h, v);
	vec4 kd = vec4(1 - vec3(ks), 1.f);
	kd *= (1 - metallic);

	float D = normal_distribution(h, n);
	float G = G_Smith(n, v, l);

	float v_dot_n = max(dot(v, n), 0.000001f);
	float l_dot_n = max(dot(l, n), 0.000001f);

	// Cook-Torrance
	vec4 specular = (ks * D * G) / (4 * v_dot_n * l_dot_n);

	vec4 brdf = kd * lambertian() + specular;

	float h_dot_n = max(dot(h, n), 0.0f);

	return  brdf * directional_light.colour * h_dot_n * directional_light.brightness;
}

void main()
{
	if (u_custom)
	{
		base_colour = u_flat_colour;
		normal = v_normal;
		metallic = u_metallic;
		roughness = u_roughness;
		ao = 0.5f;
	}
	else
	{
		//base_colour = texture(diffuse_t, v_tex_coord);
		base_colour = apply_kernel(blur_kernel, diffuse_t);
		normal = texture(normal_t, v_tex_coord).rgb;
		metallic = texture(specular_t, v_tex_coord).r;
		roughness = texture(specular_t, v_tex_coord).g;
		ao = texture(occlusion_t, v_tex_coord).r;
	}

	if (base_colour.a < 0.01f)
	{
		discard;
	}

	F0 = mix(F0, base_colour, metallic);

	vec4 ambient = vec4(0.2f, 0.2f, 0.2f, 1.f) * base_colour * ao;

	if (directional_light.active)
		colour += direct_light();

	for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
	{
		if(point_lights[i].active)
			colour += point_light(i);
	}
	
	colour += ambient;
}