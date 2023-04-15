#version 430

#extension GL_ARB_bindless_texture : require

/*----------Textures----------*/
layout (binding = 0) uniform sampler2D diffuse_t;
layout (binding = 1) uniform sampler2D specular_t;
layout (binding = 2) uniform sampler2D normal_t;
layout (binding = 3) uniform sampler2D occlusion_t;
layout (binding = 4) uniform sampler2D shadow_map_t;
layout (binding = 5) uniform samplerCube cube_map;

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

struct PointLight
{
    vec4 colour;
    vec3 position;
    float range;
    float brightness;
    bool shadow_casting;
};

uniform int u_num_point_lights;

layout (std430, binding=1) buffer Lights
{
    PointLight point_lights[];
};

layout (std430, binding=2) buffer DL
{
    DirectionalLight directional_light;
};

//layout (std430, binding=3) buffer ShadowCubeMaps
//{
//    samplerCube cube_map[];
//};

vec4 base_colour;
float spec_val;
vec3 normal;

out vec4 colour;

bool out_of_frustrum(vec3 pos)
{
    return (pos.x < -1.f || pos.x > 1.f) || (pos.y < -1.f || pos.y > 1.f) || (pos.z < -1.f || pos.z > 1.f);
}

vec4 direct_light()
{
    vec3 light_dir = normalize(directional_light.direction);
    vec3 view_dir = normalize(u_cam_pos - v_position);

    float ambient = 0.2f;
    float diffuse = max(dot(normal, light_dir), 0.0f);

    vec3 h = normalize(view_dir + light_dir);
    float spec_amount = pow(max(dot(normal, h), 0.0f), 16);


    vec3 light_pos = v_light_space_pos.xyz / v_light_space_pos.w;
    float shadow = 0.f;
    if(!out_of_frustrum(light_pos))
    {
        light_pos = (light_pos + 1.f) / 2.f;
        float shadow_bias = max(0.0002f * (1.f - dot(normal, light_dir)), 0.0005f);

        vec2 texel_size = 1.0 / textureSize(shadow_map_t, 0);

        // sample all surrounding shadow values and take the average
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float depth = texture(shadow_map_t, light_pos.xy + vec2(x, y) * texel_size).r;

                if(light_pos.z - shadow_bias > depth)
                    shadow += 1.f;
            }
        }
        shadow /= 9.0;
    }

    return vec4(((base_colour.xyz * (diffuse * (1.f - shadow) + ambient)) + spec_val * spec_amount * (1.f - shadow)) * directional_light.colour.xyz, 1.f);
}

vec4 point_light(int i)
{
    vec3 light_vec = point_lights[i].position - v_position;
    float distance = length(light_vec);
    vec3 light_dir = normalize(light_vec);
    vec3 view_dir = normalize(u_cam_pos - v_position);
    vec3 h = normalize(view_dir + light_dir);

    float shadow = 0.f;
    if(point_lights[i].shadow_casting)
    {
        vec3 fragToLight = v_position - point_lights[i].position;
        float closest_depth = texture(cube_map, fragToLight).r;
        closest_depth *= 100.f;

        float current_depth = length(fragToLight);
        float bias = 0.05;
        shadow = current_depth - bias > closest_depth ? 1.0 : 0.0;
       // return vec4(vec3(closest_depth / 100.f), 1.0);
    }

//    if (distance > point_lights[i].range)
//    {
//        return vec4(0.f);
//    }

    float attenuation = (1 / distance) * point_lights[i].range * 0.5f;
    float ambient = 0.1f;
    float diffuse = max(dot(normal, light_dir), 0.0f);

    // specular lighting
    float specular = 0.0f;
    if (diffuse != 0.0f)
    {
        specular = pow(max(dot(normal, h), 0.0f), 16) * 0.3f;
    };

    return vec4((base_colour.xyz * (diffuse * attenuation * (1.f - shadow) + ambient) + spec_val * specular * attenuation * (1.f - shadow)), 1.f) * point_lights[i].colour;
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

        if(textureSize(specular_t, 0).x > 1)
            spec_val = texture(specular_t, v_tex_coord).r;
        else
            spec_val = u_metallic;

        if(textureSize(normal_t, 0).x > 1)
            normal = normalize(vec3(texture(normal_t, v_tex_coord)));
        else
            normal = normalize(v_normal);
    }

    colour = vec4(base_colour.xyz * 0.05f, 1.f);

    if(directional_light._active)
        colour += direct_light();

    //colour += point_light(0);

    for (int i = 0; i < u_num_point_lights; ++i)
    {
        colour += point_light(i);
    }
}