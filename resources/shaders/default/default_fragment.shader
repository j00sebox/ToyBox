#version 420

/*----------Textures----------*/
layout (binding = 0) uniform sampler2D diffuse_t;
layout (binding = 1) uniform sampler2D specular_t;
layout (binding = 2) uniform sampler2D normal_t;
layout (binding = 3) uniform sampler2D occlusion_t;
layout (binding = 4) uniform sampler2D shadow_map_t;

in vec3 v_position;
in vec3 v_normal;
in vec2 v_tex_coord;
in vec4 v_light_space_pos;

uniform vec3 u_cam_pos;
uniform bool u_custom;
uniform vec4 u_base_colour;
uniform float u_metallic;

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

vec4 base_colour;
float spec_val;
vec3 normal;

out vec4 colour;

vec4 direct_light()
{
    vec3 light_dir = normalize(directional_light.direction);
    vec3 view_dir = normalize(u_cam_pos - v_position);

    float ambient = 0.2f;
    float diffuse = max(dot(normal, light_dir), 0.0f);

    float shadow = 0.f;
    float shadow_bias = max(0.00002f * (1.f - dot(normal, light_dir)), 0.0005f);
    vec3 light_pos = v_light_space_pos.xyz / v_light_space_pos.w;

    if(light_pos.z <= 1.f)
    {
        light_pos = (light_pos + 1.f) / 2.f;

        float map_depth = texture(shadow_map_t, light_pos.xy).r;

        if((light_pos.z - shadow_bias) > map_depth)
        {
            shadow = 1.f;
        }
    }

    return vec4((base_colour.xyz * (diffuse * (1.f - shadow) + ambient)) * directional_light.colour.xyz, 1.f);
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

    return (base_colour * (diffuse * attenuation + ambient) + spec_val * specular * attenuation) * point_lights[i].colour;
}

void main()
{
    if(u_custom)
    {
        base_colour = u_base_colour;
        spec_val = u_metallic;
        normal = normalize(v_normal);
    }
    else
    {
        base_colour = texture(diffuse_t, v_tex_coord);
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