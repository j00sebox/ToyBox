#version 420

/*----------Textures----------*/

layout (binding = 0) uniform sampler2D diffuse_t;
layout (binding = 1) uniform sampler2D specular_t;
layout (binding = 2) uniform sampler2D normal_t;
layout (binding = 3) uniform sampler2D occlusion_t;

in vec3 v_position;
in vec3 v_normal;
in vec2 v_tex_coord;
in mat3 v_model;

uniform bool u_custom;
uniform vec4 u_flat_colour;
uniform vec3 u_cam_pos;

uniform int u_shininess = 2;
vec4 base_colour;
vec3 normal;
float ks;
float ao;

/*----------Lighting----------*/

struct DirectionalLight
{                       // base alignment   // alignment offset
    bool active;        // 4                // 0
    vec4 colour;        // 16               // 16
    vec3 direction;     // 12               // 36
    float brightness;   // 4                // 48
}; // 48 bytes

#define MAX_POINT_LIGHTS 2

struct PointLight
{                       // base alignment   // alignment offset
    bool active;        // 4                // 0
    vec4 colour;        // 16               // 16
    vec3 position;      // 12               // 32
    float range;        // 4                // 44
    float radius;       // 4                // 48
    float brightness;   // 4                // 52
}; // 64 bytes

layout (std140, binding=1) uniform Lights
{
    PointLight point_lights[MAX_POINT_LIGHTS];  // 128 bytes
    DirectionalLight directional_light;
};

float specular_factor(vec3 n, vec3 h)
{
    return ks * pow(max(dot(n, h), 0.0), u_shininess);
}

vec4 point_light(int i)
{
    vec3 light_vec = point_lights[i].position - v_position;
    float distance = length(light_vec);
    vec3 l = normalize(light_vec);
    vec3 v = normalize(u_cam_pos - v_position);
    vec3 n = normalize(v_model * normal);
    vec3 h = normalize(l + v);

//    if (distance > point_lights[i].range)
//    {
//        return vec4(0.f);
//    }

    float attenuation = 1 / (distance * distance);

    return point_lights[i].colour * attenuation * dot(l, n) * (base_colour + specular_factor(n, h));
}

vec4 direct_light()
{
    vec3 l = normalize(directional_light.direction);
    vec3 v = normalize(u_cam_pos - v_position);
    vec3 n = normalize(v_model * normal); // TODO: will need to change once non uniform scaling is implemented
    vec3 h = normalize(l + v);

    return directional_light.colour * dot(l, n) * (base_colour + specular_factor(n, h));
}

out vec4 colour;

void main()
{
    if (u_custom)
    {
        base_colour = u_flat_colour;
        normal = v_normal;
        ks = 1.0;
        ao = 0.5f;
    }
    else
    {
        base_colour = texture(diffuse_t, v_tex_coord);
        ks = texture(specular_t, v_tex_coord).r;
        normal = texture(normal_t, v_tex_coord).rgb;
        ao = texture(occlusion_t, v_tex_coord).r;
    }

    if (base_colour.a < 0.01f)
    {
        discard;
    }

    vec4 ambient = vec4(0.2f, 0.2f, 0.2f, 1.f) * base_colour * ao;

    if (directional_light.active)
        colour += direct_light();

    for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
    {
        if(point_lights[i].active)
        {
            colour += point_light(i);
        }
    }

    colour += ambient;
}