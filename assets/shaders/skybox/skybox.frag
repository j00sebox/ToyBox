#version 450
layout(location = 0) in vec3 v_tex_coord;

layout(set = 1, binding = 0) uniform samplerCube skybox_cubemap;

layout(location = 0) out vec4 out_colour;

void main()
{
    out_colour = texture(skybox_cubemap, v_tex_coord);
}