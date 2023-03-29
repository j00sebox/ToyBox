#version 420

/*----------Textures----------*/
layout (binding = 0) uniform sampler2D diffuse_t;
layout (binding = 1) uniform sampler2D specular_t;
layout (binding = 2) uniform sampler2D normal_t;
layout (binding = 3) uniform sampler2D occlusion_t;

in vec3 v_position;
in vec3 v_normal;
in vec2 v_tex_coord;

uniform vec3 u_cam_pos;
uniform bool u_custom;
uniform float u_diffuse;
uniform float u_spec;

/*----------Lighting----------*/
struct DirectionalLight
{
    bool _active;
    vec4 colour;
    vec3 direction;
    float brightness;
};

#define MAX_POINT_LIGHTS 2

struct PointLight
{
    bool _active;
    vec4 colour;
    vec3 position;
    float range;
    float radius;
    float brightness;
};

layout (std140, binding=1) uniform Lights
{
    PointLight point_lights[MAX_POINT_LIGHTS];  // 128 bytes
    DirectionalLight directional_light;
};

vec4 diffuse_val;
float spec_val;
vec3 normal;

out vec4 colour;

vec4 direct_light()
{
    vec3 light_dir = normalize(directional_light.direction);
    vec3 view_dir = normalize(u_cam_pos - v_position);

    float ambient = 0.2f;
    float diffuse = max(dot(normal, light_dir), 0.0f);

    return (diffuse_val * (diffuse + ambient)) * directional_light.colour;
}

vec4 point_light(int i)
{
    vec3 light_vec = point_lights[i].position - v_position;
    float distance = length(light_vec);
    vec3 light_dir = normalize(light_vec);
    vec3 view_dir = normalize(u_cam_pos - v_position);
    vec3 h = normalize(view_dir + light_dir);

    if (distance > point_lights[i].range)
    {
        return vec4(0.f);
    }

    float attenuation = 1 / distance;
    float ambient = 0.2f;
    float diffuse = max(dot(normal, light_dir), 0.0f);

    // specular lighting
    float specular = 0.0f;
    if (diffuse != 0.0f)
    {
        specular = pow(max(dot(normal, h), 0.0f), 16) * 0.3f;
    };

    return (diffuse_val * (diffuse * attenuation + ambient) + spec_val * specular * attenuation) * point_lights[i].colour;
}

void main()
{
    if(u_custom)
    {
        diffuse_val = vec4(1.f, 1.f, 1.f, 1.f);
        spec_val = 0.2f;
        normal = normalize(v_normal);
    }
    else
    {
        diffuse_val = texture(diffuse_t, v_tex_coord);
        spec_val = texture(specular_t, v_tex_coord).r;
        normal = normalize(vec3(texture(normal_t, v_tex_coord)));
    }

    if (directional_light._active)
        colour += direct_light();

    for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
    {
        if(point_lights[i]._active)
        colour += point_light(i);
    }
}