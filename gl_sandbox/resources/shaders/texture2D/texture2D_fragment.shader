#version 410

uniform sampler2D diffuse_t;
uniform sampler2D specular_t;

in vec3 v_position;
in vec3 v_normal;
in vec2 v_tex_coord;

uniform vec3 u_directional_light;
uniform vec3 u_camera_position;

// point light stuff
uniform vec3 u_pl_pos;
uniform vec4 u_pl_col;
uniform float u_pl_rad;

out vec4 colour;

vec4 point_light()
{
	vec3 light_vec = u_pl_pos - v_position;
	float distance = length(light_vec);
	float attenuation = 1.f / (distance * distance);
    //float attenuation = 2.f / (distance * distance + u_pl_rad * u_pl_rad + distance * sqrt(distance * distance + u_pl_rad * u_pl_rad));

	// ambient light
	float ambient = 0.2f;

	// diffuse 
	vec3 normal = normalize(v_normal);
	vec3 direction = normalize(light_vec);
	float diffuse = max(dot(normal, direction), 0.f);
	
	return texture(diffuse_t, v_tex_coord) * (diffuse * attenuation + ambient);
}

void main()
{
	colour = point_light();
	//colour = texture(diffuse_t, v_tex_coord) ;
}